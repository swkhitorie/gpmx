#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>

#include "gpm/fs/fs.h"
#include "inode/inode.h"

/****************************************************************************
 * Name: fs_checkfd
 *
 * Description:
 *   Check if the file descriptor is valid for the provided TCB and if it
 *   supports the requested access.
 *
 ****************************************************************************/
static inline int fs_checkfd(struct tcb_s *tcb, int fd, int oflags)
{
    struct file *filep;
    struct inode *inode;
    int ret;

    ret = fs_getfilep(fd, &filep);
    if (ret < 0) {
        return ret;
    }

    inode = filep->f_inode;
    if (!inode) {
        return -ENOENT;
    }


    if (inode_checkflags(inode, oflags) != 0) {
        return -EACCES;
    }

    return 0;
}

int fs_fdopen(int fd, int oflags, struct tcb_s *tcb,
              struct file_struct **filep)
{
    struct streamlist *slist;
    FILE              *stream;
    int                ret = 0;

    if (fd < 0) {
        ret = -EBADF;
        goto errout;
    }

    if (!tcb) {
        tcb = nxsched_self();
    }

    if (fd >= 3) {
        ret = fs_checkfd(tcb, fd, oflags);
    }

    if (ret < 0) {
        goto errout;
    }

#ifdef CONFIG_MM_KERNEL_HEAP
  slist = tcb->group->tg_streamlist;
#else
  slist = &tcb->group->tg_streamlist;
#endif

  /* Allocate FILE structure */

  if (fd >= 3)
    {
      stream = group_zalloc(tcb->group, sizeof(FILE));
      if (stream == NULL)
        {
          ret = -ENOMEM;
          goto errout;
        }

      /* Add FILE structure to the stream list */

      ret = nxsem_wait(&slist->sl_sem);
      if (ret < 0)
        {
          group_free(tcb->group, stream);
          goto errout;
        }

      if (slist->sl_tail)
        {
          slist->sl_tail->fs_next = stream;
          slist->sl_tail = stream;
        }
      else
        {
          slist->sl_head = stream;
          slist->sl_tail = stream;
        }

      nxsem_post(&slist->sl_sem);

      /* Initialize the semaphore the manages access to the buffer */

      lib_sem_initialize(stream);
    }
  else
    {
      stream = &slist->sl_std[fd];
    }

#ifndef CONFIG_STDIO_DISABLE_BUFFERING
#if CONFIG_STDIO_BUFFER_SIZE > 0
  /* Set up pointers */

  stream->fs_bufstart = stream->fs_buffer;
  stream->fs_bufend   = &stream->fs_bufstart[CONFIG_STDIO_BUFFER_SIZE];
  stream->fs_bufpos   = stream->fs_bufstart;
  stream->fs_bufread  = stream->fs_bufstart;
  stream->fs_flags    = __FS_FLAG_UBF; /* Fake setvbuf and fclose */

#ifdef CONFIG_STDIO_LINEBUFFER
  /* Setup buffer flags */

  stream->fs_flags   |= __FS_FLAG_LBF; /* Line buffering */

#endif /* CONFIG_STDIO_LINEBUFFER */
#endif /* CONFIG_STDIO_BUFFER_SIZE > 0 */
#endif /* CONFIG_STDIO_DISABLE_BUFFERING */

  /* Save the file description and open flags.  Setting the
   * file descriptor locks this stream.
   */

    stream->fs_fd       = fd;
    stream->fs_oflags   = oflags;

    if (filep != NULL) {
        *filep = stream;
    }

    return 0;

errout:
    if (filep != NULL) {
        *filep = NULL;
    }

    return ret;
}
