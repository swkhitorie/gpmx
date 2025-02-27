#include <stddef.h>
#include "pthread.h"
#include "errno.h"
#include "time.h"
#include "utils.h"

#include "./prv_timer.h"

int timer_create(clockid_t clockid, FAR struct sigevent *evp, FAR timer_t *timerid)
{
    int ret = 0;
    timer_internal_t *p = NULL;

    (void)clockid;

    if (evp == NULL || evp->sigev_notify == SIGEV_SIGNAL) {
        // errno = ENOTSUP;
        ret = -1;
    }

    if (ret == 0) {
        p = pvPortMalloc(sizeof(timer_internal_t));
        if (p == NULL) {
            // errno = EAGAIN;
            ret = -1;
        }
    }

    if (ret == 0) {
        p->event = *evp;
        p->period = 0;
        *timerid = (timer_t)xTimerCreateStatic( "timer",
                                                portMAX_DELAY,
                                                pdFALSE,
                                                (void *)p,
                                                prv_timer_callback,
                                                &p->buff );
    }
    return ret;
}
