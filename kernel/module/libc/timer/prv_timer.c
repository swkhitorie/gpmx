#include "errno.h"
#include "pthread.h"
#include "time.h"

#include "./prv_timer.h"

void prv_timer_callback(TimerHandle_t handle)
{
    timer_internal_t *p = (timer_internal_t *)pvTimerGetTimerID(handle);
    pthread_t notify_thread;

    configASSERT(p != NULL);
    configASSERT(p->event.sigev_notify != SIGEV_SIGNAL);

    if (p->period > 0) {
        xTimerChangePeriod(handle, p->period, 0);
    }

    if (p->event.sigev_notify == SIGEV_THREAD) {
        if( p->event.sigev_notify_attributes == NULL) {
            (*p->event.sigev_notify_function)(p->event.sigev_value);
        } else {
            (void)pthread_create(&notify_thread,
                                p->event.sigev_notify_attributes,
                                (void *(*)(void *))p->event.sigev_notify_function,
                                p->event.sigev_value.sival_ptr);
        }
    }
}
