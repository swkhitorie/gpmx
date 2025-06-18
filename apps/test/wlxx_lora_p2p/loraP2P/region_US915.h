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

struct __region_grp;
typedef struct __region_grp region_grp_t;

struct __lora_state;
typedef struct __lora_state lora_state_t;

#ifdef __cplusplus
extern "C" {
#endif

void region_us915_init_default(region_grp_t *param);

uint8_t region_us915_downchannelnext(lora_state_t *obj);

uint8_t region_us915_upchannelnext(lora_state_t *obj);

#ifdef __cplusplus
}
#endif

#endif // REGION_US915_H
