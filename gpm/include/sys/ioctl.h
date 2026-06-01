#ifndef POSIX_IOCTL_H_
#define POSIX_IOCTL_H_

#include "gpm/fs/ioctl.h"

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

int ioctl(int fd, int req, ...);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif
