#include "rtcm_rcver.h"
#include <board_config.h>

static rtcm_t rtk_rover = {0};
static rtcm_t rtk_base = {0};
static struct timeval vt_debug;
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
            gnss_first_msg_handle();

            rt = decode_rtcm3(msg);

            if (rt == 21) {
                // use ST-EPVT for timestamp
                gnss_timestamp_get_start_calib(msg->st.now.time);
            }

            if (rt == 21) {

                // board_rtc_get_timeval(&vt_debug);

                printf("[%d] EPVT quality:%d, sat:%d, satv:%d, id:%d, tow:%d, now:%d.%03d\r\n", 
                    HAL_GetTick(), 
                    msg->st.quality, msg->st.sat_use, msg->st.sat_view, msg->st.time_id,
                    msg->st.epoch_time, (uint32_t)msg->st.now.time, (int)msg->st.now.sec);

                // printf("timestamp compare: %u, %u\r\n", (uint32_t)vt_debug.tv_sec,
                //     (uint32_t)msg->st.now.time);

                if (msg->bds_time_sync == SYS_CMP) {
                    printf("bds now: %u \r\n", (uint32_t)msg->bds_now);
                }
            }

            rf = 1;
            memcpy(rp, &msg->buff[0], msg->len+3);
            *rlen = msg->len+3;

            msg->nbyte = msg->len = 0;
            memset(msg->buff, 0, RTCM3_FRAME_MAX_BUFFER_LEN);
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
            memcpy(rp, &msg->buff[0], msg->len+3);
            *rlen = msg->len+3;

            rf = 1;

            msg->nbyte = msg->len = 0;
            memset(msg->buff, 0, RTCM3_FRAME_MAX_BUFFER_LEN);
            break;
        }
    }

    return rf;
}



/****************************************************************************
 * User RTCM Frame Encode
 ****************************************************************************/
int rtcm_imu_encode(uint8_t *p, rtcm_imu_t *rimu)
{
    int payload_len = 40;
    int idx = 14+10;
    uint32_t *pt = (uint32_t *)&rimu->now;

    p[0] = RTCM3PREAMB;
    p[1] = 0x00;
    setbitu(&p[0], 14, 10, payload_len);
    setbitu(&p[0], idx, 32, 0x20);   idx+=32;

    setbitu(&p[0], idx, 32, pt[0]);     idx+=32;
    setbitu(&p[0], idx, 32, pt[1]);     idx+=32;
    setbitu(&p[0], idx, 32, rimu->subsec);  idx+=32;
    setbits(&p[0], idx, 32, rimu->accx);    idx+=32;
    setbits(&p[0], idx, 32, rimu->accy);    idx+=32;
    setbits(&p[0], idx, 32, rimu->accz);    idx+=32;
    setbits(&p[0], idx, 32, rimu->gyrox);   idx+=32;
    setbits(&p[0], idx, 32, rimu->gyroy);   idx+=32;
    setbits(&p[0], idx, 32, rimu->gyroz);   idx+=32;

    setbitu(&p[0], idx, 24, 
        rtk_crc24q(&p[0], payload_len+3));

    return payload_len + 6;
}

int rtcm_speed_encode(uint8_t *p, rtcm_speed_t *rspeed)
{
    int payload_len = 20;
    int idx = 14+10;
    uint32_t *pt = (uint32_t *)&rspeed->now;

    p[0] = RTCM3PREAMB;
    p[1] = 0x00;
    setbitu(&p[0], 14, 10, payload_len);
    setbitu(&p[0], idx, 32, 0x30);   idx+=32;

    setbitu(&p[0], idx, 32, pt[0]);     idx+=32;
    setbitu(&p[0], idx, 32, pt[1]);     idx+=32;
    setbitu(&p[0], idx, 32, rspeed->subsec);        idx+=32;
    setbitu(&p[0], idx, 32, rspeed->vehicle_speed); idx+=32;

    setbitu(&p[0], idx, 24, 
        rtk_crc24q(&p[0], payload_len+3));

    return payload_len + 6;
}
