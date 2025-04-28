#ifndef RTCM3_H_
#define RTCM3_H_

#include "rtkcmn.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * return ->
 *  0: no head
 * -1: no enough data
 * -2: crc error
 */
int input_rtcm3(rtcm_ut *rtcm, unsigned char data);

int decode_rtcm3(rtcm_ut *rtcm);

#ifdef __cplusplus
}
#endif

#endif
