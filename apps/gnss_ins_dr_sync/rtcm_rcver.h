#ifndef RTCM_RECEIVER_H_
#define RTCM_RECEIVER_H_

#include <stdint.h>
#include <rtkcmn.h>
#include "gnss_pps_sync.h"

#ifdef __cplusplus
extern "C" {
#endif

int rtcm_rcv_process(uint8_t c);


#ifdef __cplusplus
}
#endif

#endif
