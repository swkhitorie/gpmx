#ifndef RTCM_RECEIVER_H_
#define RTCM_RECEIVER_H_

#include <stdint.h>
#include <rtkcmn.h>
#include "gnss_pps_sync.h"

#ifdef __cplusplus
extern "C" {
#endif

int rtcm_rover_process(uint8_t c, uint8_t *rp, uint16_t *rlen);

int rtcm_base_process(uint8_t c, uint8_t *rp, uint16_t *rlen);

#ifdef __cplusplus
}
#endif

#endif
