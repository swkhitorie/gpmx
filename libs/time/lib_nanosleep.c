#include "errno.h"
#include "time.h"

/* FreeRTOS interface include */
#include <FreeRTOS.h>
#include <task.h>

int nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
{
    int ret = 0;
    TickType_t sleep_time = 0;
    (void) rmtp;

    if (utils_validtimespec(rqtp) == false) {
        // errno = EINVAL;
        ret = -1;
    }

    if (ret == 0) {
        if (utils_timespec_toticks(rqtp, &sleep_time) == 0) {
            vTaskDelay( sleep_time );
        }
    }
    return ret;
}
