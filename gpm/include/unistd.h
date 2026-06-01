#ifndef POSIX_UNISTD_H_
#define POSIX_UNISTD_H_

#include <sys/types.h>

/* Values for seeking */

#define SEEK_SET    0  /* From the start of the file */
#define SEEK_CUR    1  /* From the current file offset */
#define SEEK_END    2  /* From the end of the file */

/* Bit values for the second argument to access */

#define F_OK        0  /* Test existence */
#define X_OK        1  /* Test execute permission */
#define W_OK        2  /* Test write permission */
#define R_OK        4  /* Test read permission */

/* The following symbolic constants must be defined for file streams: */

#define STDERR_FILENO                    2       /* File number of stderr */
#define STDIN_FILENO                     0       /* File number of stdin */
#define STDOUT_FILENO                    1       /* File number of stdout */

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

pid_t   gettid(void);
unsigned int sleep(unsigned int ms);
int          usleep(useconds_t usec);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif
