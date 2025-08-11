#include "rtcm_rcver.h"
#include <board_config.h>

static rtcm_t rtkmsg = {0};
int rtcm_rcv_process(uint8_t c)
{
    int rt = input_rtcm3(&rtkmsg, c);
    switch (rt) {
    case -1: break;
    case -2: break;
    case -3: break;
    case 0: {
            gnss_first_msg_handle();

            rt = decode_rtcm3(&rtkmsg);

            if (rt == 21) {
                // use ST-EPVT for timestamp
                gnss_timestamp_get_start_calib(rtkmsg.st.now.time);
            }

            if (rt == 21) {
                struct timeval vt;
                printf("[%d] quality:%d, sat:%d, satv:%d, id:%d, tow:%d, now:%d.%03d\r\n", HAL_GetTick(), 
                    rtkmsg.st.quality, rtkmsg.st.sat_use, rtkmsg.st.sat_view, rtkmsg.st.time_id,
                    rtkmsg.st.epoch_time, (uint32_t)rtkmsg.st.now.time, (int)rtkmsg.st.now.sec);

                printf("timestamp compare: %u, %u\r\n",
                    (uint32_t)board_rtc_get_timeval(&vt),
                    (uint32_t)rtkmsg.st.now.time);

                if (rtkmsg.bds_time_sync == SYS_CMP) {
                    printf("bds now: %u \r\n", (uint32_t)rtkmsg.bds_now);
                }
            }

            break;
        }
    }

    return rt;
}
