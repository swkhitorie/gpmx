#include "region_CN470.h"
#include "p2p_common.h"


void region_cn470_init_default(struct __channel_grp *grp)
{
#if defined(P2P_REGION_CN470)
    int i = 0;

    // Ping Channel
    p2p_channel_config(&grp->ping, CN470_PING_SLOT_CHANNEL_FREQ, 0, 7, 1, 18,16);

    for( uint8_t i = 0; i < CN470_MAX_NB_CHANNELS - 8; i++ ) {
        // 125 kHz channels
        p2p_channel_config(&grp->ch_list[i], 470300000 + i * 200000, 0, 7, 1, 18,16);
    }

    for( uint8_t i = CN470_MAX_NB_CHANNELS - 8; i < CN470_MAX_NB_CHANNELS; i++ ) {
        // 500 kHz channels
        p2p_channel_config(&grp->ch_list[i], 470300000 + ( i - ( CN470_MAX_NB_CHANNELS - 8 ) ) * 1600000, 2, 7, 1, 18,16);
    }

    grp->grp_ch_len = CN470_MAX_NB_CHANNELS;

    for (i = 0; i < grp->grp_ch_len; i++) {
        grp->bad_list[i] = 0;
    }

    grp->up_list = &grp->ch_list[0];
    grp->up_ch_len = 8;

    grp->dw_list = &grp->ch_list[8];
    grp->dw_ch_len = grp->grp_ch_len - 8;
#endif
}

void region_cn470_channelstate_reset(struct __channel_grp *grp)
{
#if defined(P2P_REGION_CN470)
    int i = 0;

    for (i = 0; i < grp->grp_ch_len; i++) {
        grp->bad_list[i] = 0;
    }
#endif
}

uint8_t region_cn470_downchannelnext(struct __channel_grp *grp, int16_t rssi, int8_t snr)
{
#if defined(P2P_REGION_CN470)
    int cnt_down = 0;

    if (rssi < -95 || snr < -5) {
        grp->bad_list[8 + grp->dw_fq_idx] = 1;
    }

    do {
        grp->dw_fq_idx = rand_lcg_seed_next(grp->dw_ch_len);

        if (grp->dw_list[grp->dw_fq_idx].freq < 400) {
            goto freq_error;
        }

        cnt_down++;

        if (cnt_down >= grp->dw_ch_len) {
            rand_lcg_seed_reset();
            region_eu868_channelstate_reset(grp);
            grp->dw_fq_idx = rand_lcg_seed_next(grp->dw_ch_len);

            if (grp->dw_list[grp->dw_fq_idx].freq < 100) {
                goto freq_error;
            }
            break;
        }

        if (0 == grp->bad_list[8 + grp->dw_fq_idx]) {
            break;
        }
    } while(1);

    return 0;

freq_error:
#endif

    return 1;
}

uint8_t region_cn470_upchannelnext(struct __channel_grp *grp)
{
#if defined(P2P_REGION_CN470)
    // as ping channel, use fixed up channel
#endif
    return 0;
}
