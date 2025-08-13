#ifndef REGION_EU868_H
#define REGION_EU868_H

#include <stdint.h>

/* 863Mhz ~ 870Mhz */
/* LoRa maximum number of channels */
#define EU868_MAX_NB_CHANNELS                       10

#define EU868_MIN_TX_POWER                          16
#define EU868_MAX_TX_POWER                          18
#define EU868_DEFAULT_TX_POWER                      16

/* LoRaMac default down channel 1 */
#define EU868_PING_SLOT_CHANNEL_FREQ                864525000  // loraWan:869525000

/* LoRaMac default up channel 1 */
#define EU868_LC1                                   865100000  // loraWan LC1: 868100000

/* LoRaMac default up channel 2 */
#define EU868_LC2                                   865300000  // loraWan LC2: 868300000

/* LoRaMac default up channel 3 */
#define EU868_LC3                                   865500000  // loraWan LC3: 868500000

struct __channel_grp;
typedef struct __channel_grp channel_grp_t;

struct __p2p_obj;
typedef struct __p2p_obj p2p_obj_t;

#ifdef __cplusplus
extern "C" {
#endif

void region_eu868_init_default(channel_grp_t *param);

void region_eu868_channelstate_reset(p2p_obj_t *obj);

uint8_t region_eu868_downchannelnext(p2p_obj_t *obj, int16_t rssi, int8_t snr);

uint8_t region_eu868_upchannelnext(p2p_obj_t *obj);

#ifdef __cplusplus
}
#endif

#endif // __REGION_EU868_H__
