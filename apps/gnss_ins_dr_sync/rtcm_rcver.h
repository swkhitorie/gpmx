#ifndef RTCM_RECEIVER_H_
#define RTCM_RECEIVER_H_

#include <stdint.h>
#include <rtkcmn.h>
#include "gnss_pps_sync.h"

typedef struct __rtcm_imu {
    uint32_t id;         //0x20
    time_t now;
    uint32_t subsec;
    int32_t accx;
    int32_t accy;
    int32_t accz;
    int32_t gyrox;
    int32_t gyroy;
    int32_t gyroz;
} rtcm_imu_t;

typedef struct __rtcm_speed {
    uint32_t id;         //0x30
    time_t now;
    uint32_t subsec;

    uint32_t vehicle_speed;
} rtcm_speed_t;

#ifdef __cplusplus
extern "C" {
#endif

int rtcm_rcv_process(uint8_t c, uint8_t *rp, uint16_t *rlen);

int rtcm_imu_encode(uint8_t *p, rtcm_imu_t *rimu);

int rtcm_speed_encode(uint8_t *p, rtcm_speed_t *rspeed);

#ifdef __cplusplus
}
#endif

#endif
