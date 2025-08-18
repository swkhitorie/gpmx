#include "rtcm_rcver.h"
#include <board_config.h>

static rtcm_t rtkmsg = {0};
int rtcm_rcv_process(uint8_t c, uint8_t *rp, uint16_t *rlen)
{
    int rf = 0;

    int rt = input_rtcm3(&rtkmsg, c);
    switch (rt) {
    case -1: rf = 2; break;
    case -2: rf = 3; break;
    case -3: rf = 4; break;
    case 0: {
            gnss_first_msg_handle();

            rt = decode_rtcm3(&rtkmsg);

            if (rt == 21) {
                // use ST-EPVT for timestamp
                gnss_timestamp_get_start_calib(rtkmsg.st.now.time);
            }

            if (rt == 21) {
                struct timeval vt;

                printf("[%d] EPVT quality:%d, sat:%d, satv:%d, id:%d, tow:%d, now:%d.%03d\r\n", 
                    HAL_GetTick(), 
                    rtkmsg.st.quality, rtkmsg.st.sat_use, rtkmsg.st.sat_view, rtkmsg.st.time_id,
                    rtkmsg.st.epoch_time, (uint32_t)rtkmsg.st.now.time, (int)rtkmsg.st.now.sec);

                printf("timestamp compare: %u, %u\r\n",
                    (uint32_t)board_rtc_get_timeval(&vt),
                    (uint32_t)rtkmsg.st.now.time);

                if (rtkmsg.bds_time_sync == SYS_CMP) {
                    printf("bds now: %u \r\n", (uint32_t)rtkmsg.bds_now);
                }
            }

            rf = 1;
            memcpy(rp, &rtkmsg.buff[0], rtkmsg.len+3);
            *rlen = rtkmsg.len+3;
            break;
        }
    }

    return rf;
}

int rtcm_imu_encode(uint8_t *p, rtcm_imu_t *rimu)
{
    int payload_len = 36;
    int idx = 14+10;
    p[0] = RTCM3PREAMB;
    p[1] = 0x00;
    setbitu(&p[0], 14, 10, payload_len);

    setbits(&p[0], idx, 64, rimu->now);     idx+=64;
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
    int payload_len = 16;
    int idx = 14+10;
    p[0] = RTCM3PREAMB;
    p[1] = 0x00;
    setbitu(&p[0], 14, 10, payload_len);

    setbits(&p[0], idx, 64, rspeed->now);           idx+=64;
    setbitu(&p[0], idx, 32, rspeed->subsec);        idx+=32;
    setbitu(&p[0], idx, 32, rspeed->vehicle_speed); idx+=32;

    setbitu(&p[0], idx, 24, 
        rtk_crc24q(&p[0], payload_len+3));

    return payload_len + 6;
}
