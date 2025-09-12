#include "gnss_pps_sync.h"

#include <board_config.h>
/** chip running tick, unit in ms */
#define GNSS_PPS_SYNC_TIMESTAMP()    HAL_GetTick()

#include <drivers/drv_hrt.h>
#include <stdio.h>
#include "app_main.h"
struct __gnsspps_timesync {
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
static uint32_t calib_time;

static uint32_t hrt_pps_timestamp_lst = 0;
static uint32_t hrt_pps_timestamp = 0;
static uint32_t hrt_pps_err_acc = 0;
static uint32_t hrt_pps_err = 0;

static uint32_t syst_pps_timestamp_lst = 0;
static uint32_t syst_pps_timestamp = 0;
static uint32_t syst_pps_err_acc = 0;
static uint32_t syst_pps_err = 0;
static volatile struct __gnsspps_timesync tpps = {
    .pps_cnt = 0,
    .pps_stable_cnt = 3,
    .pps_time = 0,
    .gnss_msg_next_time = 0,
    .err_pps_msg_delay = 0,
    .hrt_rtc_start_calib_time = 0,
    .hrt_rtc_calib_flag = false,
    .step_pps_msg = 0x01,
};

void gnsspps_reset()
{
    tpps.pps_cnt = 0;
    tpps.pps_stable_cnt = 3;
    tpps.pps_time = 0;
    tpps.gnss_msg_next_time = 0;
    tpps.err_pps_msg_delay = 0;
    tpps.hrt_rtc_start_calib_time = 0;
    tpps.hrt_rtc_calib_flag = false;
    tpps.step_pps_msg = 0x01;
}

void hrt_pps_calib_callback()
{
    hrt_init();
    board_rtc_set_time_stamp(tpps.hrt_rtc_start_calib_time);

    syst_pps_timestamp_lst = HAL_GetTick();

    calib_time = HAL_GetTick();
    printf("[sync] (%d) calib rtc and hrt %d, %d\r\n", 
        HAL_GetTick(),
        tpps.hrt_rtc_start_calib_time,
        (uint32_t)hrt_absolute_time());
}

void gnss_time_debug()
{
    printf("[sync] (%d) pps delay: %d, calib: %d\r\n", 
        HAL_GetTick(),
        tpps.err_pps_msg_delay,
        tpps.hrt_rtc_calib_flag);
}

void gnss_time_debug2(time_t t1)
{
    printf("[sync] (%d) get rtcm now: %u next calib: %u\r\n", 
        HAL_GetTick(),
        (uint32_t)t1,
        (uint32_t)t1+1);
}

void pps_pulse_irq()
{
    if (tpps.hrt_rtc_calib_flag == 1) {
        hrt_pps_timestamp = hrt_absolute_time();
        syst_pps_timestamp = HAL_GetTick();

        hrt_pps_err = hrt_pps_timestamp - hrt_pps_timestamp_lst - 1000000;
        hrt_pps_err_acc += hrt_pps_err;
        hrt_pps_timestamp_lst = hrt_pps_timestamp;

        syst_pps_err = syst_pps_timestamp - syst_pps_timestamp_lst - 1000;
        syst_pps_err_acc += syst_pps_err;
        syst_pps_timestamp_lst = syst_pps_timestamp;

        printf("[pps ] (%d) err hrt: %d,%d, sys: %d,%d, tag:%x\r\n", HAL_GetTick(),
            hrt_pps_err_acc, hrt_pps_err, 
            syst_pps_err_acc, syst_pps_err, run_tag);
    }

    if (tpps.hrt_rtc_calib_flag == 0) {
        hrt_pps_err = 0;
        hrt_pps_err_acc = 0;
        hrt_pps_timestamp_lst = 0;

        syst_pps_err = 0;
        syst_pps_err_acc = 0;
        syst_pps_timestamp_lst = 0;
    }

    if (!tpps.hrt_rtc_calib_flag &&
        tpps.hrt_rtc_start_calib_time != 0) {
        tpps.hrt_rtc_calib_flag = true;

        hrt_pps_calib_callback();
    }

    tpps.pps_cnt++;
    if (tpps.pps_cnt > tpps.pps_stable_cnt) {
        tpps.pps_time = GNSS_PPS_SYNC_TIMESTAMP();

        if (tpps.step_pps_msg == 0x01) {
            tpps.step_pps_msg = 0x02;
        }
    }
}

void gnsspps_afterpps_fstmsg()
{
    if (tpps.step_pps_msg == 0x02) {
        tpps.step_pps_msg = 0x01;
        tpps.gnss_msg_next_time = GNSS_PPS_SYNC_TIMESTAMP();
        tpps.err_pps_msg_delay = tpps.gnss_msg_next_time - tpps.pps_time;

        if (!tpps.hrt_rtc_calib_flag) {
            tpps.step_pps_msg = 0x03;
        } else {
            gnss_time_debug();
        }
    }
}

void gnsspps_calibhrt_timestamp(time_t now)
{
    if (tpps.step_pps_msg == 0x03 && 
        tpps.hrt_rtc_start_calib_time == 0) {

        tpps.step_pps_msg = 0x01;
        tpps.hrt_rtc_start_calib_time = now+1;

        gnss_time_debug2(now);
    }
}

void gnsspps_timestamp(struct timeval *now)
{
    int32_t hrt_subsec = 0;
    uint32_t hrt_timestamp = 0;

    time_t time_now = board_rtc_get_timeval(now);
    now->tv_sec = time_now;

    if (!tpps.hrt_rtc_calib_flag) {
        return;
    }

    hrt_timestamp = hrt_absolute_time();
    hrt_subsec = (int32_t)(hrt_timestamp%1000000);

    if (hrt_subsec - (int32_t)now->tv_usec < 0) {
        // hrt time overflow, but rtc not yet
        now->tv_sec += 1;

        // printf("[sync] (%d) hrt overflow comp: %d.%d - %d.%d\r\n", HAL_GetTick(),
        //     (uint32_t)tpps.hrt_rtc_start_calib_time+(hrt_timestamp/1000000),
        //     (uint32_t)hrt_subsec,
        //     (uint32_t)now->tv_sec,
        //     (uint32_t)now->tv_usec);
    }

    now->tv_usec = hrt_subsec;
}

void gnsspps_recalib()
{
    if (calib_time > 0 && (HAL_GetTick() - calib_time >= 60*1000)) {
        calib_time = 0;
        gnsspps_reset();
    }
}

uint32_t gnss_get_dbg1()
{
    uint32_t abs_time = hrt_absolute_time()/1000000;
    return abs_time + tpps.hrt_rtc_start_calib_time;
}

uint32_t gnss_get_dbg2()
{
    uint32_t abs_subtime = hrt_absolute_time()%1000000;
    return abs_subtime;
}
