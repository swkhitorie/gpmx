#ifndef POSIX_MQUEUE_H_
#define POSIX_MQUEUE_H_

#include "sys/types.h"
#include "time.h"

struct mq_attr
{
    size_t         mq_maxmsg;    /* Max number of messages in queue */
    size_t         mq_msgsize;   /* Max message size */
    unsigned       mq_flags;     /* Queue flags */
    size_t         mq_curmsgs;   /* Number of messages currently in queue */
};

//typedef FAR struct void* mqd_t;
typedef FAR void* mqd_t;

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

int     mq_close        (mqd_t mqdes);
int     mq_getattr      (mqd_t mqdes, struct mq_attr *mqstat);
mqd_t   mq_open         (const char *name, int oflag, mode_t mode, struct mq_attr *attr);
ssize_t mq_receive      (mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);
int     mq_send         (mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio);
ssize_t mq_timedreceive (mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned *msg_prio, const struct timespec *abstime);
int     mq_timedsend    (mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio, const struct timespec *abstime);
int     mq_unlink       (const char *name);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif
