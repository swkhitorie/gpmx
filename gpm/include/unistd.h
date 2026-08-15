#ifndef __INCLUDE_UNISTD_H
#define __INCLUDE_UNISTD_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <sys/types.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

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

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/* Task Control Interfaces */

unsigned int sleep(unsigned int ms);
int          usleep(useconds_t usec);

/* File descriptor operations */

int     close(int fd);
int     dup(int fd);
int     dup2(int fd1, int fd2);
int     fsync(int fd);
off_t   lseek(int fd, off_t offset, int whence);
ssize_t read(int fd, void *buf, size_t nbytes);
ssize_t write(int fd, const void *buf, size_t nbytes);

/* Special devices */

int     pipe(int fd[2]);
int     pipe2(int pipefd[2], int flags);

/* File path operations */

int     rmdir(const char *pathname);
int     unlink(const char *pathname);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __INCLUDE_UNISTD_H */
