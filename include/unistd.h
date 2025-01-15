#ifndef POSIX_UNISTD_H_
#define POSIX_UNISTD_H_

#include "sys/types.h"

#define F_OK        0  /* Test existence */
#define X_OK        1  /* Test execute permission */
#define W_OK        2  /* Test write permission */
#define R_OK        4  /* Test read permission */

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

unsigned int sleep(unsigned int ms);
int     usleep(useconds_t usec);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif
