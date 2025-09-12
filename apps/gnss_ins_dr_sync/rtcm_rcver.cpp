#include "rtcm_rcver.h"
#include <board_config.h>

static rtcm_t rtk_rover = {0};
static rtcm_t rtk_base = {0};
struct timeval dbg_rtc;
int rtcm_rover_process(uint8_t c, uint8_t *rp, uint16_t *rlen)
{
    int rf = 0;
    rtcm_t *msg = &rtk_rover;

    int rt = input_rtcm3(msg, c);
    switch (rt) {
    case -1: rf = 2; break;
    case -2: rf = 3; break;
    case -3: rf = 4; break;
    case 0: {
            gnsspps_afterpps_fstmsg();

            rt = decode_rtcm3(msg);

            if (rt == 21) {
                // use ST-EPVT for timestamp
                gnsspps_calibhrt_timestamp(msg->st.now.time);

                board_rtc_get_timeval(&dbg_rtc);
                printf("[rtcm] (%d) EPVT quality:%d, sat:%d, satv:%d, id:%d, tow:%d, now:%d.%03d, bds_now:%d, hrt:%d.%06d, rtc:%d.%06d\r\n", 
                    HAL_GetTick(), 
                    msg->st.quality, 
                    msg->st.sat_use, 
                    msg->st.sat_view, 
                    msg->st.time_id,
                    msg->st.epoch_time, 
                    (uint32_t)msg->st.now.time, 
                    (int)msg->st.now.sec, 
                    (uint32_t)msg->bds_now,
                    gnss_get_dbg1(),
                    gnss_get_dbg2(),
                    (uint32_t)dbg_rtc.tv_sec,
                    dbg_rtc.tv_usec);
            }

            *rlen = msg->len+3;
            memcpy(rp, &msg->buff[0], msg->len+3);

            msg->nbyte = msg->len = 0;
            memset(msg->buff, 0, RTCM3_FRAME_MAX_BUFFER_LEN);

            rf = 1;
            break;
        }
    }

    return rf;
}

int rtcm_base_process(uint8_t c, uint8_t *rp, uint16_t *rlen)
{
    int rf = 0;
    rtcm_t *msg = &rtk_base;

    int rt = input_rtcm3(msg, c);
    switch (rt) {
    case -1: rf = 2; break;
    case -2: rf = 3; break;
    case -3: rf = 4; break;
    case 0: {

            *rlen = msg->len+3;
            memcpy(rp, &msg->buff[0], msg->len+3);

            msg->nbyte = msg->len = 0;
            memset(msg->buff, 0, RTCM3_FRAME_MAX_BUFFER_LEN);

            rf = 1;
            break;
        }
    }

    return rf;
}

