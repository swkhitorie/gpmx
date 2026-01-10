#include "gromfs.h"

inline int check_dirent(struct romfs_dirent *dirent)
{
    if (dirent == NULL
        ||(dirent->type != ROMFS_DIRENT_FILE && dirent->type != ROMFS_DIRENT_DIR)
        || dirent->size == ~0)
        return -1;
    return 0;
}

struct romfs_dirent *dfs_romfs_lookup(struct romfs_dirent *root_dirent, const char *path, size_t *size)
{
    size_t index, found;
    const char *subpath, *subpath_end;
    struct romfs_dirent *dirent;
    size_t dirent_size;

    /* Check the root_dirent. */
    if (check_dirent(root_dirent) != 0)
        return NULL;

    if (path[0] == '/' && path[1] == '\0') {
        *size = root_dirent->size;
        return root_dirent;
    }

    /* goto root directory entries */
    dirent = (struct romfs_dirent *)root_dirent->data;
    dirent_size = root_dirent->size;

    /* get the end position of this subpath */
    subpath_end = path;
    /* skip /// */
    while (*subpath_end && *subpath_end == '/')
        subpath_end ++;
    subpath = subpath_end;
    while ((*subpath_end != '/') && *subpath_end)
        subpath_end ++;

    while (dirent != NULL)
    {
        found = 0;

        /* search in folder */
        for (index = 0; index < dirent_size; index ++)
        {
            if (check_dirent(&dirent[index]) != 0)
                return NULL;
            if (strlen(dirent[index].name) ==  (size_t)(subpath_end - subpath) &&
                    strncmp(dirent[index].name, subpath, (subpath_end - subpath)) == 0)
            {
                dirent_size = dirent[index].size;

                /* skip /// */
                while (*subpath_end && *subpath_end == '/')
                    subpath_end ++;
                subpath = subpath_end;
                while ((*subpath_end != '/') && *subpath_end)
                    subpath_end ++;

                if (!(*subpath))
                {
                    *size = dirent_size;
                    return &dirent[index];
                }

                if (dirent[index].type == ROMFS_DIRENT_DIR)
                {
                    /* enter directory */
                    dirent = (struct romfs_dirent *)dirent[index].data;
                    found = 1;
                    break;
                }
                else
                {
                    /* return file dirent */
                    return &dirent[index];
                }
            }
        }

        if (!found)
            break; /* not found */
    }

    /* not found */
    return NULL;
}

size_t dfs_romfs_read(struct dfs_file *file, void *buf, size_t count, uint32_t *pos)
{
    size_t length;
    struct romfs_dirent *dirent;

    dirent = (struct romfs_dirent *)file->data;
    if (check_dirent(dirent) != 0) {
        return -5;
    }

    if (count < dirent->size - *pos)
        length = count;
    else
        length = dirent->size - *pos;

    if (length > 0)
        memcpy(buf, &(dirent->data[*pos]), length);

    /* update file current position */
    *pos += length;

    return length;
}

int dfs_romfs_close(struct dfs_file *file)
{
    return 0;
}

int dfs_romfs_open(struct dfs_file *file, const char *path)
{
    size_t size;
    struct romfs_dirent *dirent;
    struct romfs_dirent *root_dirent;

    if (!file) {
        return -1;
    }

    root_dirent = (struct romfs_dirent *)&romfs_root;
    if (check_dirent(root_dirent) != 0) {
        return -5;
    }

    /* get rom dirent */
    dirent = dfs_romfs_lookup(root_dirent, path, &size);
    if (dirent == NULL) {
        return -6;
    }

    file->data = dirent;
    return 0;
}

int dfs_romfs_stat(struct dfs_file *file, struct dfs_fstat *st)
{
    // int ret = dfs_file_lock();
    struct romfs_dirent *dirent = (struct romfs_dirent *)file->data;

    st->st_size = dirent->size;

    // dfs_file_unlock();
    return 0;
}

// int dfs_romfs_getdents(struct dfs_file *file, struct dirent *dirp, uint32_t count)
// {
//     rt_size_t index;
//     const char *name;
//     struct dirent *d;
//     struct romfs_dirent *dirent, *sub_dirent;

//     dirent = (struct romfs_dirent *)file->data;
//     if (check_dirent(dirent) != 0 || dirent->type != ROMFS_DIRENT_DIR) {
//         return -1;
//     }

//     /* enter directory */
//     dirent = (struct romfs_dirent *)dirent->data;

//     /* make integer count */
//     count = (count / sizeof(struct dirent));
//     if (count == 0) {
//         return -2;
//     }

//     index = 0;
//     for (index = 0; index < count && file->fpos < file->vnode->size; index++)
//     {
//         d = dirp + index;

//         sub_dirent = &dirent[file->fpos];
//         name = sub_dirent->name;

//         /* fill dirent */
//         if (sub_dirent->type == ROMFS_DIRENT_DIR)
//             d->d_type = DT_DIR;
//         else
//             d->d_type = DT_REG;

//         d->d_namlen = rt_strlen(name);
//         d->d_reclen = (rt_uint16_t)sizeof(struct dirent);
//         rt_strncpy(d->d_name, name, DIRENT_NAME_MAX);

//         /* move to next position */
//         ++ file->fpos;
//     }

//     return index * sizeof(struct dirent);
// }


int dfs_romfs_init()
{
    /* register rom file system */
    // dfs_register(&_romfs);

    return 0;
}

static const unsigned char _dummy_dummy_txt[] =
{
    0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x61, 0x20, 0x66, 0x69, 0x6c, 0x65, 0x21, 0x0d, 0x0a,
};

static const struct romfs_dirent _dummy[] =
{
    {ROMFS_DIRENT_FILE, "dummy.txt", _dummy_dummy_txt, sizeof(_dummy_dummy_txt)},
};

static const unsigned char _dummy_txt[] =
{
    0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x61, 0x20, 0x66, 0x69, 0x6c, 0x65, 0x21, 0x0d, 0x0a,
};

#if defined(__clang__) || defined(__GNUC__)
__attribute__((weak)) const struct romfs_dirent _root_dirent[] =
#else
__weak const struct romfs_dirent _root_dirent[] =
#endif
{
    {ROMFS_DIRENT_DIR, "dev", NULL, 0},
    {ROMFS_DIRENT_DIR, "mnt", NULL, 0},
    {ROMFS_DIRENT_DIR, "proc", NULL, 0},
    {ROMFS_DIRENT_DIR, "etc", NULL, 0},
    {ROMFS_DIRENT_DIR, "bin", NULL, 0},
    {ROMFS_DIRENT_DIR, "dummy", (uint8_t *)_dummy, sizeof(_dummy) / sizeof(_dummy[0])},
    {ROMFS_DIRENT_FILE, "dummy.txt", _dummy_txt, sizeof(_dummy_txt)},
};

#if defined(__clang__) || defined(__GNUC__)
__attribute__((weak)) const struct romfs_dirent romfs_root =
#else
__weak const struct romfs_dirent romfs_root =
#endif
{
    ROMFS_DIRENT_DIR, "/", (uint8_t *)_root_dirent, sizeof(_root_dirent) / sizeof(_root_dirent[0])
};

