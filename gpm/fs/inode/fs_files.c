#include <sys/types.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>

#include <semaphore.h>

#include "gpm/fs/fs.h"
#include "inode/inode.h"

#define CONFIG_FDCLONE_STDIO

static int _files_semtake(struct filelist *list)
{
    return sem_wait(&list->fl_sem);
}

#define _files_semgive(list) sem_post(&list->fl_sem)

static int files_extend(struct filelist *list, size_t row)
{
    struct file **tmp;
    int i;

    if (row <= list->fl_rows) {
        return 0;
    }

    if (row * CONFIG_NFILE_DESCRIPTORS_PER_BLOCK > OPEN_MAX) {
        return -EMFILE;
    }

    tmp = kmm_realloc(list->fl_files, sizeof(struct file *) * row);
    if (tmp == NULL) {
        return -ENFILE;
    }

    i = list->fl_rows;
    do {
        tmp[i] = kmm_zalloc(sizeof(struct file) *
                            CONFIG_NFILE_DESCRIPTORS_PER_BLOCK);
        if (tmp[i] == NULL) {
            while (--i >= list->fl_rows) {
                kmm_free(tmp[i]);
            }

            kmm_free(tmp);
            return -ENFILE;
        }
    } while (++i < row);

    list->fl_files = tmp;
    list->fl_rows = row;

    return 0;
}

/****************************************************************************
 * Name: files_initlist
 *
 * Description: Initializes the list of files for a new task
 *
 ****************************************************************************/
void files_initlist(struct filelist *list)
{
    sem_init(&list->fl_sem, 0, 1);
}

/****************************************************************************
 * Name: files_releaselist
 *
 * Description:
 *   Release a reference to the file list
 *
 ****************************************************************************/
void files_releaselist(struct filelist *list)
{
    int i;
    int j;

    /* Close each file descriptor .. Normally, you would need take the list
    * semaphore, but it is safe to ignore the semaphore in this context
    * because there should not be any references in this context.
    */
    for (i = list->fl_rows - 1; i >= 0; i--) {
        for (j = CONFIG_NFILE_DESCRIPTORS_PER_BLOCK - 1; j >= 0; j--) {
            file_close(&list->fl_files[i][j]);
        }
        kmm_free(list->fl_files[i]);
    }

    kmm_free(list->fl_files);
    sem_destroy(&list->fl_sem);
}

/****************************************************************************
 * Name: files_allocate
 *
 * Description:
 *   Allocate a struct files instance and associate it with an inode
 *   instance.  Returns the file descriptor == index into the files array.
 *
 ****************************************************************************/
int files_allocate(struct inode *inode, int oflags, off_t pos,
                   void *priv, int minfd)
{
    struct filelist *list;
    int ret;
    int i;
    int j;

    /* Get the file descriptor list.  It should not be NULL in this context. */
    list = sched_get_files();

    ret = _files_semtake(list);
    if (ret < 0) {
        return ret;
    }

    /* Calculate minfd whether is in list->fl_files.
    * if not, allocate a new filechunk.
    */
    i = minfd / CONFIG_NFILE_DESCRIPTORS_PER_BLOCK;
    if (i >= list->fl_rows) {
        ret = files_extend(list, i + 1);
        if (ret < 0) {
            _files_semgive(list);
            return ret;
        }
    }

    /* Find free file */
    j = minfd % CONFIG_NFILE_DESCRIPTORS_PER_BLOCK;
    do {
        do {
            if (!list->fl_files[i][j].f_inode) {
                list->fl_files[i][j].f_oflags = oflags;
                list->fl_files[i][j].f_pos    = pos;
                list->fl_files[i][j].f_inode  = inode;
                list->fl_files[i][j].f_priv   = priv;
                _files_semgive(list);
                return i * CONFIG_NFILE_DESCRIPTORS_PER_BLOCK + j;
            }
        } while (++j < CONFIG_NFILE_DESCRIPTORS_PER_BLOCK);
        j = 0;
    } while (++i < list->fl_rows);

    ret = files_extend(list, i + 1);
    if (ret >= 0) {
        list->fl_files[i][0].f_oflags = oflags;
        list->fl_files[i][0].f_pos    = pos;
        list->fl_files[i][0].f_inode  = inode;
        list->fl_files[i][0].f_priv   = priv;
        ret = i * CONFIG_NFILE_DESCRIPTORS_PER_BLOCK;
    }

    _files_semgive(list);
    return ret;
}

/****************************************************************************
 * Name: files_duplist
 *
 * Description:
 *   Duplicate parent task's file descriptors.
 *
 ****************************************************************************/
int files_duplist(struct filelist *plist, struct filelist *clist)
{
    int ret;
    int i;
    int j;

    ret = _files_semtake(plist);
    if (ret < 0) {
        return ret;
    }

    for (i = 0; i < plist->fl_rows; i++) {
        for (j = 0; j < CONFIG_NFILE_DESCRIPTORS_PER_BLOCK; j++) {
            struct file *filep;
#ifdef CONFIG_FDCLONE_STDIO

            /* Determine how many file descriptors to clone.  If
            * CONFIG_FDCLONE_DISABLE is set, no file descriptors will be
            * cloned.  If CONFIG_FDCLONE_STDIO is set, only the first
            * three descriptors (stdin, stdout, and stderr) will be
            * cloned.  Otherwise all file descriptors will be cloned.
            */

            if (i * CONFIG_NFILE_DESCRIPTORS_PER_BLOCK + j >= 3) {
                goto out;
            }
#endif

            filep = &plist->fl_files[i][j];
            if (filep->f_inode == NULL || (filep->f_oflags & O_CLOEXEC) != 0) {
                continue;
            }

            ret = files_extend(clist, i + 1);
            if (ret < 0) {
                goto out;
            }

            /* Yes... duplicate it for the child */
            ret = file_dup2(filep, &clist->fl_files[i][j]);
            if (ret < 0) {
                goto out;
            }
        }
    }

out:
    _files_semgive(plist);
    return ret;
}

/****************************************************************************
 * Name: fs_getfilep
 *
 * Description:
 *   Given a file descriptor, return the corresponding instance of struct
 *   file.
 *
 * Input Parameters:
 *   fd    - The file descriptor
 *   filep - The location to return the struct file instance
 *
 * Returned Value:
 *   Zero (OK) is returned on success; a negated errno value is returned on
 *   any failure.
 *
 ****************************************************************************/
int fs_getfilep(int fd, struct file **filep)
{
    struct filelist *list;
    int ret;

    *filep = (struct file *)NULL;

    list = sched_get_files();
    if (list == NULL) {
        return -EAGAIN;
    }

    if (fd < 0 || fd >= list->fl_rows * CONFIG_NFILE_DESCRIPTORS_PER_BLOCK) {
        return -EBADF;
    }

    ret = _files_semtake(list);
    if (ret < 0) {
        return ret;
    }

    *filep = &list->fl_files[fd / CONFIG_NFILE_DESCRIPTORS_PER_BLOCK]
                            [fd % CONFIG_NFILE_DESCRIPTORS_PER_BLOCK];

    if (!(*filep)->f_inode) {
        *filep = (struct file *)NULL;
        ret = -EBADF;
    }

    _files_semgive(list);
    return ret;
}

/****************************************************************************
 * Name: nx_dup2
 *
 * Description:
 *   nx_dup2() is similar to the standard 'dup2' interface except that is
 *   not a cancellation point and it does not modify the errno variable.
 *
 *   nx_dup2() is an internal NuttX interface and should not be called from
 *   applications.
 *
 *   Clone a file descriptor to a specific descriptor number.
 *
 * Returned Value:
 *   fd2 is returned on success; a negated errno value is return on
 *   any failure.
 *
 ****************************************************************************/
int nx_dup2(int fd1, int fd2)
{
    struct filelist *list;
    int ret;

    list = sched_get_files();

    if (fd1 < 0 || fd1 >= CONFIG_NFILE_DESCRIPTORS_PER_BLOCK * list->fl_rows ||
        fd2 < 0)
    {
        return -EBADF;
    }

    ret = _files_semtake(list);
    if (ret < 0) {
        return ret;
    }

    if (fd2 >= CONFIG_NFILE_DESCRIPTORS_PER_BLOCK * list->fl_rows) {
        ret = files_extend(list, fd2 / CONFIG_NFILE_DESCRIPTORS_PER_BLOCK + 1);
        if (ret < 0) {
            _files_semgive(list);
            return ret;
        }
    }

    ret = file_dup2(&list->fl_files[fd1 / CONFIG_NFILE_DESCRIPTORS_PER_BLOCK]
                                    [fd1 % CONFIG_NFILE_DESCRIPTORS_PER_BLOCK],
                    &list->fl_files[fd2 / CONFIG_NFILE_DESCRIPTORS_PER_BLOCK]
                                    [fd2 % CONFIG_NFILE_DESCRIPTORS_PER_BLOCK]);
    _files_semgive(list);
    return ret < 0 ? ret : fd2;
}

int dup2(int fd1, int fd2)
{
    int ret;

    ret = nx_dup2(fd1, fd2);
    if (ret < 0) {
        set_errno(-ret);
        ret = -1;
    }

    return ret;
}

int nx_close(int fd)
{
    struct filelist *list;
    struct file     *filep;
    struct file      file;
    int                  ret;

    list = sched_get_files();

    ret = _files_semtake(list);
    if (ret < 0) {
        return ret;
    }

    if (fd < 0 || fd >= list->fl_rows * CONFIG_NFILE_DESCRIPTORS_PER_BLOCK ||
        !list->fl_files[fd / CONFIG_NFILE_DESCRIPTORS_PER_BLOCK]
                        [fd % CONFIG_NFILE_DESCRIPTORS_PER_BLOCK].f_inode)
    {
        _files_semgive(list);
        return -EBADF;
    }

    filep = &list->fl_files[fd / CONFIG_NFILE_DESCRIPTORS_PER_BLOCK]
                            [fd % CONFIG_NFILE_DESCRIPTORS_PER_BLOCK];
    memcpy(&file, filep, sizeof(struct file));
    memset(filep, 0,     sizeof(struct file));

    _files_semgive(list);

    return file_close(&file);
}

int close(int fd)
{
    int ret;

    ret = nx_close(fd);
    if (ret < 0) {
        set_errno(-ret);
        ret = -1;
    }

    return ret;
}
