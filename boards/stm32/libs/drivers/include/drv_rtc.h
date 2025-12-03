#ifndef DRIVE_RTC_H_
#define DRIVE_RTC_H_

#include <stdint.h>
#include <stdbool.h>
#include "drv_cmn.h"
#include <device/rtc.h>

#ifdef cplusplus
extern "C" {
#endif

bool   hw_stm32_rtc_setup(void);
void   hw_stm32_rtc_deinit(void);
rclk_time_t hw_stm32_rtc_get_timeval(struct rclk_timeval *tv);
void        hw_stm32_rtc_get_tm(struct rclk_tm *now);
bool        hw_stm32_rtc_set_time_stamp(rclk_time_t time_stamp); 

#ifdef cplusplus
}
#endif

#endif
