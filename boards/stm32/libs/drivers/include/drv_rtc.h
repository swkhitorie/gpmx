#ifndef DRV_RTC_H_
#define DRV_RTC_H_

#include <stdint.h>
#include "drv_common.h"
#include <sys/time.h>

#ifdef cplusplus
extern "C" {
#endif

bool low_rtc_setup(void);

time_t low_rtc_get_timeval(struct timeval *tv);

/** UTC Timestamp from 1970/01/01 00:00:00 */
bool low_rtc_set_time_stamp(time_t time_stamp);

#ifdef cplusplus
}
#endif

#endif
