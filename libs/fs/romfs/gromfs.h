/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019/01/13     Bernard      code cleanup
 */

#ifndef __GROMFS_H__
#define __GROMFS_H__

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define ROMFS_DIRENT_FILE   0x00
#define ROMFS_DIRENT_DIR    0x01

struct romfs_dirent
{
    uint32_t      type;  /* dirent type */

    const char    *name; /* dirent name */
    const uint8_t *data; /* file date ptr */
    size_t        size;  /* file size */
};

struct dfs_fstat
{
    uint32_t  st_size;
};

struct dfs_dirent
{
    uint8_t  d_type;             /* The type of the file */
    uint8_t  d_namlen;           /* The length of the not including the terminating null file name */
    uint16_t d_reclen;           /* length of this record */
    char d_name[128];   /* The null-terminated file name */
};

struct dfs_file
{
    uint16_t magic;              /* file descriptor magic number */
    uint32_t flags;              /* Descriptor flags */
    int ref_count;               /* Descriptor reference count */
    uint32_t  pos;                /* Current file position */
    void  *data;                  /* Specific fd data */
};

#ifdef __cplusplus
extern "C" {
#endif

int dfs_romfs_init(void);
int dfs_romfs_open(struct dfs_file *file, const char *path);
int dfs_romfs_close(struct dfs_file *file);
int dfs_romfs_stat(struct dfs_file *file, struct dfs_fstat *st);
size_t dfs_romfs_read(struct dfs_file *file, void *buf, size_t count, uint32_t *pos);

#ifdef __cplusplus
}
#endif

extern const struct romfs_dirent romfs_root;

#endif
