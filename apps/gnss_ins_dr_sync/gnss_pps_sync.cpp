#include "gnss_pps_sync.h"

#include <board_config.h>
#include <drivers/drv_hrt.h>
#include <stdio.h>

/**
 * chip running tick, unit in ms
 */
#define GNSS_PPS_SYNC_TIMESTAMP()    HAL_GetTick()

struct __gnss_pps_timesync {
    uint32_t pps_cnt;
    uint32_t pps_stable_cnt;
    uint32_t pps_time;
    uint32_t gnss_msg_next_time;
    uint32_t err_pps_msg_delay;
    uint32_t hrt_rtc_start_calib_time;
    bool hrt_rtc_calib_flag;
    uint8_t step_pps_msg;
    uint8_t __pad[2];
};

static volatile struct __gnss_pps_timesync sync_pps = {
    .pps_cnt = 0,
    .pps_stable_cnt = 3,
    .pps_time = 0,
    .gnss_msg_next_time = 0,
    .err_pps_msg_delay = 0,
    .hrt_rtc_start_calib_time = 0,
    .hrt_rtc_calib_flag = false,
    .step_pps_msg = 0x01,
};

void hrt_pps_calib_callback()
{
    printf("calib rtc and hrt %d\r\n", sync_pps.hrt_rtc_start_calib_time);
    hrt_init();
    board_rtc_set_time_stamp(sync_pps.hrt_rtc_start_calib_time);
}

void gnss_time_debug()
{
    // struct tm now_t1;
    // board_rtc_get_tm(&now_t1);
    // printf("%02d:%02d:%02d.%03d\r\n",
    //     now_t1.tm_hour+8, now_t1.tm_min, now_t1.tm_sec,
    //     (hrt_absolute_time()%1000000)/1000);

    printf("pps delay: %d, calib: %d\r\n", sync_pps.err_pps_msg_delay, sync_pps.hrt_rtc_calib_flag);
}

void gnss_time_debug2(time_t t1)
{
    printf("get rtcm now: %u next calib: %u\r\n", 
        (uint32_t)t1,
        (uint32_t)t1+1);
}

void pps_pulse_irq()
{
    sync_pps.pps_cnt++;
    if (sync_pps.pps_cnt > sync_pps.pps_stable_cnt) {
        sync_pps.pps_time = GNSS_PPS_SYNC_TIMESTAMP();

        if (sync_pps.step_pps_msg == 0x01) {
            sync_pps.step_pps_msg = 0x02;
        }

        if (!sync_pps.hrt_rtc_calib_flag &&
            sync_pps.hrt_rtc_start_calib_time != 0) {
            sync_pps.hrt_rtc_calib_flag = true;

            hrt_pps_calib_callback();
        }
    }
}

void gnss_first_msg_handle()
{
    if (sync_pps.step_pps_msg == 0x02) {
        sync_pps.step_pps_msg = 0x01;
        sync_pps.gnss_msg_next_time = GNSS_PPS_SYNC_TIMESTAMP();
        sync_pps.err_pps_msg_delay = sync_pps.gnss_msg_next_time - sync_pps.pps_time;

        if (!sync_pps.hrt_rtc_calib_flag) {
            sync_pps.step_pps_msg = 0x03;
        } else {
            gnss_time_debug();
        }
    }
}

void gnss_timestamp_get_start_calib(time_t gnss_msg_utc_time)
{
    if (sync_pps.step_pps_msg == 0x03 && 
        sync_pps.hrt_rtc_start_calib_time == 0) {

        sync_pps.step_pps_msg = 0x01;
        sync_pps.hrt_rtc_start_calib_time = gnss_msg_utc_time+1;

        gnss_time_debug2(gnss_msg_utc_time);
    }
}

void gnss_hrt_timestamp_get(struct timeval *now)
{
    int32_t subsec = 0;
    time_t time_now = board_rtc_get_timeval(now);
    now->tv_sec = time_now;

    if (!sync_pps.hrt_rtc_calib_flag) {
        return;
    }

    subsec = (int32_t)(hrt_absolute_time()%1000000);

    if (subsec - (int32_t)now->tv_usec < 0) {
        // hrt time overflow, but rtc not yet
        now->tv_sec += 1;
        printf("hrt time overflow: %d, %d \r\n", subsec, now->tv_usec);
    }

    now->tv_usec = subsec;
}

