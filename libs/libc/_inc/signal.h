#ifndef POSIX_SIGNAL_H_
#define POSIX_SIGNAL_H_

#include <stdint.h>
#include "pthread.h"

#define SIGEV_NONE      0  /* No asynchronous notification is delivered */
#define SIGEV_SIGNAL    1  /* Notify via signal,with an application-defined value */
#define SIGEV_THREAD    2  /* A notification function is called (3) */

union sigval
{
    int       sival_int;       /* Integer value */
    FAR void* sival_ptr;       /* Pointer value */
};

typedef void (*sigev_notify_function_t)(union sigval value);

struct sigevent
{
    uint8_t      sigev_notify; /* Notification method: SIGEV_SIGNAL, SIGEV_NONE, or SIGEV_THREAD */
    uint8_t      sigev_signo;  /* Notification signal */
    union sigval sigev_value;  /* Data passed with notification */
    sigev_notify_function_t sigev_notify_function; /* Notification function */
    FAR pthread_attr_t *sigev_notify_attributes;   /* Notification attributes */
};

#endif
