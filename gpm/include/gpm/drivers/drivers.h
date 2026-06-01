#ifndef __INCLUDE_DRIVERS_H
#define __INCLUDE_DRIVERS_H

#include <sys/types.h>
#include <stdbool.h>

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

void drivers_initialize(void);

void devnull_register(void);

int bchdev_register(const char *blkdev, const char *chardev,
                    bool readonly);

int bchdev_unregister(const char *chardev);

int bchlib_setup(const char *blkdev, bool readonly, void **handle);

int bchlib_teardown(void *handle);

ssize_t bchlib_read(void *handle, char *buffer, size_t offset,
                    size_t len);

ssize_t bchlib_write(void *handle, const char *buffer, size_t offset,
                    size_t len);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif
