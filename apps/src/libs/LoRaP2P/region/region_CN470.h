#ifndef REGION_CN470_H
#define REGION_CN470_H

#include <stdint.h>

/* 470MHz ~ 510MHz / 470.3MHz ~ 509.7MHz */
#define CN470_MAX_NB_CHANNELS                        48

#define CN470_PING_SLOT_CHANNEL_FREQ                504900000


struct __channel_grp;
struct __p2p_obj;

#ifdef __cplusplus
extern "C" {
#endif

void region_cn470_init_default(struct __channel_grp *grp);

void region_cn470_channelstate_reset(struct __channel_grp *grp);

uint8_t region_cn470_downchannelnext(struct __channel_grp *grp, int16_t rssi, int8_t snr);

uint8_t region_cn470_upchannelnext(struct __channel_grp *grp);

#ifdef __cplusplus
}
#endif

#endif // __REGION_EU868_H__
