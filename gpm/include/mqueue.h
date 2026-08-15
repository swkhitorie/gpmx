#ifndef __INCLUDE_MQUEUE_H
#define __INCLUDE_MQUEUE_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <gpmx/config.h>
#include <sys/types.h>
#include <time.h>

/********************************************************************************
 * Public Type Definitions
 ********************************************************************************/

struct mq_attr
{
    size_t         mq_maxmsg;    /* Max number of messages in queue */
    size_t         mq_msgsize;   /* Max message size */
    unsigned       mq_flags;     /* Queue flags */
    size_t         mq_curmsgs;   /* Number of messages currently in queue */
};

#if defined(CONFIG_FREERTOS_ENABLE)

typedef void* mqd_t;
#elif defined(CONFIG_RTTNANO_ENABLE)

typedef int mqd_t;
#endif

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/********************************************************************************
 * Public Function Prototypes
 ********************************************************************************/

int     mq_close        (mqd_t mqdes);
int     mq_getattr      (mqd_t mqdes, struct mq_attr *mqstat);
mqd_t   mq_open         (const char *name, int oflag, mode_t mode, struct mq_attr *attr);
ssize_t mq_receive      (mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);
int     mq_send         (mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio);
int     mq_setattr      (mqd_t mqdes, const struct mq_attr *mqstat,struct mq_attr *omqstat);
ssize_t mq_timedreceive (mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned *msg_prio, const struct timespec *abstime);
int     mq_timedsend    (mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio, const struct timespec *abstime);
int     mq_unlink       (const char *name);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* __INCLUDE_LIBGEN_H */
