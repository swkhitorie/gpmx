#ifndef REGION_US915_H
#define REGION_US915_H

#include <stdint.h>

/* LoRa maximum number of channels */
#define US915_MAX_NB_CHANNELS                       72

/* Ping slot channel frequency */
#define US915_PING_SLOT_CHANNEL_FREQ                923300000

#define US915_MIN_TX_POWER                          16
#define US915_MAX_TX_POWER                          18
#define US915_DEFAULT_TX_POWER                      16

struct __channel_grp;
struct __p2p_obj;

#ifdef __cplusplus
extern "C" {
#endif

void region_us915_init_default(struct __channel_grp *grp);

void region_us915_channelstate_reset(struct __channel_grp *grp);

uint8_t region_us915_downchannelnext(struct __channel_grp *grp, int16_t rssi, int8_t snr);

uint8_t region_us915_upchannelnext(struct __channel_grp *grp);

#ifdef __cplusplus
}
#endif

#endif // REGION_US915_H
