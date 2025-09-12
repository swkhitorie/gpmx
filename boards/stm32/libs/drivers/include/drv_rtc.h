#ifndef DRV_RTC_H_
#define DRV_RTC_H_

#include <stdint.h>
#include "drv_common.h"

#if defined(__clang__) || defined(__CC_ARM)
#include <time.h>
struct timeval {
    time_t tv_sec;
    int tv_usec;
};
#elif defined(__GNUC__)
#include <sys/time.h>
#else
#error drv_rtc: unkown compiler, support: gae, armclang, armcc
#endif

#ifdef cplusplus
extern "C" {
#endif

bool   stm32_rtc_setup(void);
time_t stm32_rtc_get_timeval(struct timeval *tv);
void   stm32_rtc_get_tm(struct tm *now);

/** UTC Timestamp from 1970/01/01 00:00:00 */
bool   stm32_rtc_set_time_stamp(time_t time_stamp); 


#ifdef cplusplus
}
#endif

#endif
