#include "region_EU868.h"
#include "p2p_common.h"

/**
    上行链路频率 (MHz) - 设备→网关
    信道	中心频率	带宽 (BW)	扩频因子 (SF)	编码率 (CR)	最大占空比
    0	    868.10	    125 kHz	    SF7-SF12	    4/5 - 4/8	≤1%
    1	    868.30	    125 kHz	    SF7-SF12	    4/5 - 4/8	≤1%
    2	    868.50	    125 kHz	    SF7-SF12	    4/5 - 4/8	≤1%
    3	    867.10	    125 kHz	    SF7-SF12	    4/5 - 4/8	≤1%
    4	    867.30	    125 kHz	    SF7-SF12	    4/5 - 4/8	≤1%
    5	    867.50	    125 kHz	    SF7-SF12	    4/5 - 4/8	≤1%
    6	    867.70	    125 kHz	    SF7-SF12	    4/5 - 4/8	≤1%
    7	    867.90	    125 kHz	    SF7-SF12	    4/5 - 4/8	≤1%
    8*	    868.80	    125 kHz	    SF7-SF12	    4/5 - 4/8	≤0.1%
    9*	    869.525	    125 kHz	    SF7-SF12	    4/5 - 4/8	≤10%
    *注：信道8-9为特殊应用信道(如Class B信标)

    下行链路频率 (MHz) - 网关→设备
    类型	中心频率	带宽 (BW)	扩频因子 (SF)	编码率 (CR)
    RX1	    同上行	    125 kHz	    SF7-SF12	    4/5 - 4/8
    RX2	    869.525	    125 kHz 	SF9 (固定)	    4/8
*/

void region_eu868_init_default(struct __channel_grp *grp)
{
#if defined(P2P_REGION_EU868)
    int i = 0;

    // // ping channel, duty cycle < 10%
    // p2p_channel_config(&param[0], EU868_PING_SLOT_CHANNEL_FREQ, 0, 7, 1, 16);

    // // 3 standard channels, duty cycle < 1%
    // p2p_channel_config(&param[1], EU868_LC1, 0, 7, 1, 16);
    // p2p_channel_config(&param[2], EU868_LC2, 0, 7, 1, 16);
    // p2p_channel_config(&param[3], EU868_LC3, 0, 7, 1, 16);

    // // 867.1Mhz ~ 867.9Mhz, duty cycle < 1%
    // p2p_channel_config(&param[4], 867100000, 0, 7, 1, 16);
    // p2p_channel_config(&param[5], 867300000, 0, 7, 1, 16);
    // p2p_channel_config(&param[6], 867500000, 0, 7, 1, 16);
    // p2p_channel_config(&param[7], 867700000, 0, 7, 1, 16);
    // p2p_channel_config(&param[8], 867900000, 0, 7, 1, 16);

    // //Class B channel, duty cycle < 0.1%
    // p2p_channel_config(&param[9], 868800000, 0, 7, 1, 16);

    // ping channel, duty cycle < 10%
    p2p_channel_config(&grp->ping, EU868_PING_SLOT_CHANNEL_FREQ, 1, 6, 1, 18,16);

    // 3 standard channels, duty cycle < 1%
    p2p_channel_config(&grp->ch_list[0], EU868_LC1, 1, 6, 1, 18,16);
    p2p_channel_config(&grp->ch_list[1], EU868_LC2, 1, 6, 1, 18,16);
    p2p_channel_config(&grp->ch_list[2], EU868_LC3, 1, 6, 1, 18,16);

    // loraWan: 867.1 867.3 ~ 867.9
    // 867.1Mhz ~ 867.9Mhz, duty cycle < 1%
    p2p_channel_config(&grp->ch_list[3], 866100000, 1, 6, 1, 18,16);
    p2p_channel_config(&grp->ch_list[4], 866300000, 1, 6, 1, 18,16);
    p2p_channel_config(&grp->ch_list[5], 866500000, 1, 6, 1, 18,16);
    p2p_channel_config(&grp->ch_list[6], 866700000, 1, 6, 1, 18,16);
    p2p_channel_config(&grp->ch_list[7], 866900000, 1, 6, 1, 18,16);

    // loraWan: 868.8
    //Class B channel, duty cycle < 0.1%
    p2p_channel_config(&grp->ch_list[8], 863800000, 1, 6, 1, 18,16);

    grp->grp_ch_len = 9;

    for (i = 0; i < grp->grp_ch_len; i++) {
        grp->bad_list[i] = 0;
    }

    grp->up_list = &grp->ch_list[0];
    grp->up_ch_len = 3;

    grp->dw_list = &grp->ch_list[3];
    grp->dw_ch_len = grp->grp_ch_len - 3;
#endif

}

void region_eu868_channelstate_reset(struct __channel_grp *grp)
{
#if defined(P2P_REGION_EU868)
    int i = 0;

    for (i = 0; i < grp->grp_ch_len; i++) {
        grp->bad_list[i] = 0;
    }
#endif
}

uint8_t region_eu868_downchannelnext(struct __channel_grp *grp, int16_t rssi, int8_t snr)
{
#if defined(P2P_REGION_EU868)
    int cnt_down = 0;

    if (rssi < -95 || snr < -5) {
        grp->bad_list[3 + grp->dw_fq_idx] = 1;
    }

    do {
        grp->dw_fq_idx = rand_lcg_seed_next(grp->dw_ch_len);

        if (grp->dw_list[grp->dw_fq_idx].freq < 800) {
            goto freq_error;
        }

        cnt_down++;

        if (cnt_down >= grp->dw_ch_len) {
            rand_lcg_seed_reset();
            region_eu868_channelstate_reset(grp);
            grp->dw_fq_idx = rand_lcg_seed_next(grp->dw_ch_len);

            if (grp->dw_list[grp->dw_fq_idx].freq < 800) {
                goto freq_error;
            }
            break;
        }

        if (0 == grp->bad_list[3 + grp->dw_fq_idx]) {
            break;
        }
    } while(1);

    return 0;

freq_error:
#endif

    return 1;
}

uint8_t region_eu868_upchannelnext(struct __channel_grp *grp)
{
#if defined(P2P_REGION_EU868)
    // as ping channel, use fixed up channel
#endif
    return 0;
}
