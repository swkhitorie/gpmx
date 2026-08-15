#include <gpmx/config.h>
#include <time.h>
#include <errno.h>
#include "utils.h"

#if defined(CONFIG_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <task.h>
#endif

int nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
{
    int ret = 0;
#if defined(CONFIG_FREERTOS_ENABLE)
    TickType_t sleep_time = 0;
#endif

    (void)rmtp;

    if (utils_validtimespec(rqtp) == false) {
        errno = EINVAL;
        ret = -1;
    }

    if (ret == 0) {
#if defined(CONFIG_FREERTOS_ENABLE)
        if (utils_timespec_toticks(rqtp, &sleep_time) == 0) {
            vTaskDelay( sleep_time );
        }
#else
        ret = -1;
#endif
    }

    return ret;
}
