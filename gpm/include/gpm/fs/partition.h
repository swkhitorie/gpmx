#ifndef INCLUDE_FS_PARTITION_H
#define INCLUDE_FS_PARTITION_H

#include <limits.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct partition_s {
    char   name[NAME_MAX + 1];
    size_t index;
    size_t firstblock;
    size_t nblocks;
    size_t blocksize;
};

typedef void (*partition_handler_t)(struct partition_s *part, void *arg);

int parse_block_partition(  const char *path,
                            partition_handler_t handler,
                            void *arg);

struct mtd_dev_s;
int parse_mtd_partition(struct mtd_dev_s *mtd,
                        partition_handler_t handler,
                        void *arg);

#if defined(__cplusplus)
}
#endif

#endif
