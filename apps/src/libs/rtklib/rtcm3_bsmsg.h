#ifndef RTCM3_BASE_STATION_MSG_H_
#define RTCM3_BASE_STATION_MSG_H_

#include "rtkcmn.h"

#ifndef RTK_BASEMSG_BUFFER_LEN
#define RTK_BASEMSG_BUFFER_LEN      (6)
#endif

#define RTK_BASEMSG_ARP_LEN         (27)
#define RTK_BASEMSG_1013_LEN        (15)

typedef struct __rtk_msm_msg {
    uint8_t buffer[1400];
    uint16_t len;
    uint32_t tow;

    uint16_t snd_bytes;
} rtk_msm_t;

typedef struct __rtk_basestation_msg {
    uint8_t msg_arp[40];
    uint8_t len_arp;
    uint8_t snd_bytes_arp;

    uint8_t msg_1013[50];
    uint8_t len_1013;
    uint8_t snd_bytes_1013;

    rtk_msm_t current_msm;
    rtk_msm_t decoding_msm;

    rtk_msm_t msg[RTK_BASEMSG_BUFFER_LEN];
    uint8_t idx;

} rtk_basemsg_t;


#ifdef __cplusplus
extern "C" {
#endif

void basemsmbuffer_print(rtk_msm_t *buff, uint8_t capacity);

uint8_t basemsmbuffer_size(rtk_msm_t *buff, uint8_t capacity);

void basemsmbuffer_clear(rtk_msm_t *buff, uint8_t capacity);

void basemsmbuffer_peek(rtk_msm_t *buff, uint8_t capacity);

bool basemsmbuffer_add(rtk_msm_t *node, rtk_msm_t *buff, uint8_t capacity);

bool basemsmbuffer_fifoin(rtk_msm_t *node, rtk_msm_t *buff, uint8_t capacity);

bool basemsmbuffer_newest(rtk_msm_t *node, rtk_msm_t *buff, uint8_t capacity);

bool basemsmbuffer_towmatch(rtk_msm_t *node, uint32_t tow, rtk_msm_t *buff, uint8_t capacity);


void rtk_basemsg_decode(rtk_basemsg_t *baseobj, rtcm_t *rtcm, uint8_t *buffer, uint32_t len);

void rtk_basemsg_msm_decode(rtk_basemsg_t *baseobj, rtcm_t *rtcm);

#ifdef __cplusplus
}
#endif


#endif
