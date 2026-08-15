#include <gpmx/config.h>
#include <stddef.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>

#include "utils.h"
#include "prv_timer.h"

int timer_create(clockid_t clockid, struct sigevent *evp, timer_t *timerid)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    int ret = 0;
    timer_internal_t *p = NULL;

    (void)clockid;

    if (evp == NULL || evp->sigev_notify == SIGEV_SIGNAL) {
        errno = ENOTSUP;
        ret = -1;
    }

    if (ret == 0) {
        p = pvPortMalloc(sizeof(timer_internal_t));
        if (p == NULL) {
            errno = EAGAIN;
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
#else

    return -1;
#endif
}
