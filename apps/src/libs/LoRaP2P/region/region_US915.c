#include "region_US915.h"
#include "p2p_common.h"

/**
    上行链路频率 (MHz) - 设备→网关
    子带	信道范围	频率公式	         带宽 (BW)	扩频因子 (SF)	编码率 (CR)	最大占空比
    1	    0-7	        902.3 + 0.2×k	    125 kHz	    SF7-SF10	4/5 - 4/8	≤0.4%
    2	    8-15	    902.9 + 0.2×(k-8)	125 kHz	    SF7-SF10	4/5 - 4/8	≤0.4%
    3	    16-23	    903.5 + 0.2×(k-16)	125 kHz	    SF7-SF10	4/5 - 4/8	≤0.4%
    4	    24-31	    904.1 + 0.2×(k-24)	125 kHz	    SF7-SF10	4/5 - 4/8	≤0.4%
    5	    32-39	    904.7 + 0.2×(k-32)	125 kHz	    SF7-SF10	4/5 - 4/8	≤0.4%
    6	    40-47	    905.3 + 0.2×(k-40)	125 kHz	    SF7-SF10	4/5 - 4/8	≤0.4%
    7	    48-55	    905.9 + 0.2×(k-48)	125 kHz	    SF7-SF10	4/5 - 4/8	≤0.4%
    8	    56-63	    906.5 + 0.2×(k-56)	125 kHz	    SF7-SF10	4/5 - 4/8	≤0.4%
    k = 信道号 (0-63)
    示例：信道0=902.3MHz, 信道1=902.5MHz, ..., 信道63=914.9MHz

    下行链路频率 (MHz) - 网关→设备
    类型	信道	中心频率	        带宽 (BW)	扩频因子 (SF)	编码率 (CR)
    RX1	    0-7	    923.3 + 0.6×m	    500 kHz	    SF7-SF12	4/5 - 4/8
    RX2	    N/A	    923.3	            500 kHz	    SF12 (固定)	4/8
    m = 下行信道号 (0-7)
    示例：下行信道0=923.3MHz, 信道1=923.9MHz, ..., 信道7=927.5MHz
*/

void region_us915_init_default(struct __channel_grp *grp)
{
#if (!defined(LORAP2P_SAVE)) || (defined(P2P_REGION_US915) && defined(LORAP2P_SAVE))
    int i = 0;

    // Ping Channel
    p2p_channel_config(&grp->ping, US915_PING_SLOT_CHANNEL_FREQ, 0, 7, 1, 18,16);

    for( uint8_t i = 0; i < US915_MAX_NB_CHANNELS - 8; i++ ) {
        // 125 kHz channels
        p2p_channel_config(&grp->ch_list[i], 902300000 + i * 200000, 0, 7, 1, 18,16);
    }

    for( uint8_t i = US915_MAX_NB_CHANNELS - 8; i < US915_MAX_NB_CHANNELS; i++ ) {
        // 500 kHz channels
        p2p_channel_config(&grp->ch_list[i], 903000000 + ( i - ( US915_MAX_NB_CHANNELS - 8 ) ) * 1600000, 2, 7, 1, 18,16);
    }

    grp->grp_ch_len = US915_MAX_NB_CHANNELS;

    for (i = 0; i < grp->grp_ch_len; i++) {
        grp->bad_list[i] = 0;
    }

    grp->up_list = &grp->ch_list[0];
    grp->up_ch_len = 8;

    grp->dw_list = &grp->ch_list[8];
    grp->dw_ch_len = grp->grp_ch_len - 8;
#endif
}

void region_us915_channelstate_reset(struct __channel_grp *grp)
{
#if (!defined(LORAP2P_SAVE)) || (defined(P2P_REGION_US915) && defined(LORAP2P_SAVE))
    int i = 0;

    for (i = 0; i < grp->grp_ch_len; i++) {
        grp->bad_list[i] = 0;
    }
#endif
}

uint8_t region_us915_downchannelnext(struct __channel_grp *grp, int16_t rssi, int8_t snr)
{
#if (!defined(LORAP2P_SAVE)) || (defined(P2P_REGION_US915) && defined(LORAP2P_SAVE))
    int cnt_down = 0;
    if (rssi < -90 || snr < -6) {
        grp->bad_list[8 + grp->dw_fq_idx] = 1;
    }

    do {
        grp->dw_fq_idx = rand_lcg_seed_next(grp->dw_ch_len);

        cnt_down++;
        if (cnt_down >= grp->dw_ch_len) {
            rand_lcg_seed_reset();
            region_us915_channelstate_reset(grp);
            grp->dw_fq_idx = rand_lcg_seed_next(grp->dw_ch_len);
            break;
        }
        if (0 == grp->bad_list[8 + grp->dw_fq_idx]) {
            break;
        }
    } while(1);

#endif

    return 0;
}

uint8_t region_us915_upchannelnext(struct __channel_grp *grp)
{
#if (!defined(LORAP2P_SAVE)) || (defined(P2P_REGION_US915) && defined(LORAP2P_SAVE))
    // as ping channel, use fixed up channel
#endif

    return 0;
}
