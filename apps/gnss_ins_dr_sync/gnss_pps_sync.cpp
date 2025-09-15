#include "gnss_pps_sync.h"

#include <board_config.h>
#include <drivers/drv_hrt.h>
#include <stdio.h>
#include "app_main.h"

/** chip running tick, unit in ms */
#define GNSS_PPS_SYNC_TIMESTAMP()    HAL_GetTick()

#define SYNC_PPS_STABLE_TIMES              (4)

#define SYNC_STATE_IDLE                         (0x01)
#define SYNC_GET_DELAY_FROM_PPS_MSG             (0x02)
#define SYNC_WAIT_GET_PPS_UTC                   (0x03)

struct __gnsspps_timesync {
    uint8_t state;

    uint32_t pps_cnt;
    uint32_t pps_time;
    uint32_t gnss_msg_time;
    uint32_t delay_pps_to_msg;

    uint32_t hrt_rtc_calib_tick;
    uint32_t hrt_rtc_calib_times;
    uint32_t hrt_rtc_calib_timestamp;
    uint32_t hrt_rtc_calib_timestamp_tmp;
    bool     hrt_rtc_calib_flag;
};

static volatile struct __gnsspps_timesync tpps = {
    .state = SYNC_STATE_IDLE,

    .pps_cnt = 0,
    .pps_time = 0,
    .gnss_msg_time = 0,
    .delay_pps_to_msg = 0,

    .hrt_rtc_calib_tick = 0,
    .hrt_rtc_calib_times = 0,
    .hrt_rtc_calib_timestamp = 0,
    .hrt_rtc_calib_timestamp_tmp = 0,
    .hrt_rtc_calib_flag = false,
};

static uint32_t calib_time;

static uint32_t _dbg_hrt_rtc_calib_done = 0;
static uint32_t _dbg_hrt_pps_timestamp = 0;
static uint32_t _dbg_hrt_pps_err_acc = 0;
static uint32_t _dbg_hrt_pps_err = 0;

static uint32_t _dbg_syst_pps_timestamp_lst = 0;
static uint32_t _dbg_syst_pps_timestamp = 0;
static uint32_t _dbg_syst_pps_err_acc = 0;
static uint32_t _dbg_syst_pps_err = 0;


void gnsspps_reset()
{
    tpps.state = SYNC_STATE_IDLE;

    tpps.pps_cnt = 0;
    tpps.pps_time = 0;
    tpps.gnss_msg_time = 0;
    tpps.delay_pps_to_msg = 0;

    tpps.hrt_rtc_calib_tick = 0;
    tpps.hrt_rtc_calib_times = 0;
    tpps.hrt_rtc_calib_timestamp = 0;
    tpps.hrt_rtc_calib_timestamp_tmp = 0;
    tpps.hrt_rtc_calib_flag = false;
}

void hrt_pps_calib_callback()
{
    hrt_init();
    board_rtc_set_time_stamp(tpps.hrt_rtc_calib_timestamp);
    tpps.hrt_rtc_calib_tick = HAL_GetTick();
    tpps.hrt_rtc_calib_timestamp_tmp = tpps.hrt_rtc_calib_timestamp;
    tpps.hrt_rtc_calib_flag = true;
    tpps.hrt_rtc_calib_times++;


    _dbg_syst_pps_timestamp_lst = HAL_GetTick();
    printf("[sync] (%d) calib rtc and hrt %d, %d\r\n", 
        HAL_GetTick(),
        tpps.hrt_rtc_calib_timestamp,
        (uint32_t)hrt_absolute_time());
}

void gnss_time_debug()
{
    printf("[sync] (%d) pps delay: %d\r\n", 
        HAL_GetTick(),
        tpps.delay_pps_to_msg);
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
    if (tpps.hrt_rtc_calib_flag) {
        _dbg_hrt_pps_timestamp = hrt_absolute_time();
        _dbg_syst_pps_timestamp = HAL_GetTick();

        _dbg_hrt_pps_err = _dbg_hrt_pps_timestamp - _dbg_hrt_rtc_calib_done - 1000000;
        _dbg_hrt_pps_err_acc += _dbg_hrt_pps_err;
        _dbg_hrt_rtc_calib_done = _dbg_hrt_pps_timestamp;

        _dbg_syst_pps_err = _dbg_syst_pps_timestamp - _dbg_syst_pps_timestamp_lst - 1000;
        _dbg_syst_pps_err_acc += _dbg_syst_pps_err;
        _dbg_syst_pps_timestamp_lst = _dbg_syst_pps_timestamp;

        printf("[pps ] (%d) err hrt: %d,%d, sys: %d,%d, tag:%x\r\n", HAL_GetTick(),
            _dbg_hrt_pps_err_acc, _dbg_hrt_pps_err, 
            _dbg_syst_pps_err_acc, _dbg_syst_pps_err, run_tag);
    }

    if (!tpps.hrt_rtc_calib_flag) {
        _dbg_hrt_pps_err = 0;
        _dbg_hrt_pps_err_acc = 0;
        _dbg_hrt_rtc_calib_done = 0;

        _dbg_syst_pps_err = 0;
        _dbg_syst_pps_err_acc = 0;
        _dbg_syst_pps_timestamp_lst = 0;
    }

    if (!tpps.hrt_rtc_calib_flag &&
        tpps.hrt_rtc_calib_timestamp_tmp != tpps.hrt_rtc_calib_timestamp) {
        hrt_pps_calib_callback();
    }

    if (++tpps.pps_cnt > SYNC_PPS_STABLE_TIMES) {
        tpps.pps_time = GNSS_PPS_SYNC_TIMESTAMP();

        if (tpps.state == SYNC_STATE_IDLE) {
            tpps.state = SYNC_GET_DELAY_FROM_PPS_MSG;
        }
    }
}

void gnsspps_afterpps_fstmsg()
{
    if (tpps.state == SYNC_GET_DELAY_FROM_PPS_MSG) {

        tpps.gnss_msg_time = GNSS_PPS_SYNC_TIMESTAMP();
        tpps.delay_pps_to_msg = tpps.gnss_msg_time - tpps.pps_time;

        if (!tpps.hrt_rtc_calib_flag) {
            tpps.state = SYNC_WAIT_GET_PPS_UTC;
            return;
        } else {
            gnss_time_debug();
        }

        tpps.state = SYNC_STATE_IDLE;
    }
}

void gnsspps_calibhrt_timestamp(time_t now)
{
    if (tpps.state == SYNC_WAIT_GET_PPS_UTC && 
        !tpps.hrt_rtc_calib_flag) {

        tpps.state = SYNC_STATE_IDLE;
        tpps.hrt_rtc_calib_timestamp_tmp = tpps.hrt_rtc_calib_timestamp;
        tpps.hrt_rtc_calib_timestamp = now+1;

        gnss_time_debug2(now);
    }
}

void gnsspps_timestamp(struct timeval *now)
{
    int32_t hrt_subsec = 0;
    uint32_t hrt_timestamp = 0;

    if (tpps.hrt_rtc_calib_times == 0) {
        time_t time_now = board_rtc_get_timeval(now);
        now->tv_sec = time_now;
    }

    if (tpps.hrt_rtc_calib_times > 0) {

        time_t abs_time = hrt_absolute_time()/1000000;
        if (tpps.hrt_rtc_calib_flag) {
            abs_time += tpps.hrt_rtc_calib_timestamp;
        } else {
            abs_time += tpps.hrt_rtc_calib_timestamp_tmp;
        }

        now->tv_sec = abs_time;
        now->tv_usec = hrt_absolute_time()%1000000;
    }

    // time_t time_now = board_rtc_get_timeval(now);
    // now->tv_sec = time_now;

    // if (!tpps.hrt_rtc_calib_flag) {
    //     return;
    // }

    // hrt_timestamp = hrt_absolute_time();
    // hrt_subsec = (int32_t)(hrt_timestamp%1000000);

    // if (hrt_subsec - (int32_t)now->tv_usec < 0) {
    //     // hrt time overflow, but rtc not yet
    //     now->tv_sec += 1;

    //     // printf("[sync] (%d) hrt overflow comp: %d.%d - %d.%d\r\n", HAL_GetTick(),
    //     //     (uint32_t)tpps.hrt_rtc_calib_time+(hrt_timestamp/1000000),
    //     //     (uint32_t)hrt_subsec,
    //     //     (uint32_t)now->tv_sec,
    //     //     (uint32_t)now->tv_usec);
    // }

    // now->tv_usec = hrt_subsec;
}

void gnsspps_recalib()
{
    if (tpps.hrt_rtc_calib_tick > 0 && 
        (HAL_GetTick() - tpps.hrt_rtc_calib_tick >= 60*1000)) 
    {
        tpps.hrt_rtc_calib_tick = 0;
        tpps.hrt_rtc_calib_flag = false;
    }
}

uint32_t gnss_get_dbg1()
{
    uint32_t abs_time = hrt_absolute_time()/1000000;
    if (tpps.hrt_rtc_calib_flag) {
        abs_time += tpps.hrt_rtc_calib_timestamp;
    } else {
        abs_time += tpps.hrt_rtc_calib_timestamp_tmp;
    }
    return abs_time;
}

uint32_t gnss_get_dbg2()
{
    uint32_t abs_subtime = hrt_absolute_time()%1000000;
    return abs_subtime;
}
