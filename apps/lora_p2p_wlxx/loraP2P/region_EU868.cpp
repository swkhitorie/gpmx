#include "region_EU868.h"
#include "p2p_common.h"

static uint8_t eu868_p2p_upchannel_idx = 0xff;

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

void region_eu868_init_default(channel_grp_t *param)
{
    // // ping channel, duty cycle < 10%
    // p2p_channel_cfg(&param[0], EU868_PING_SLOT_CHANNEL_FREQ, 0, 7, 1, 16);

    // // 3 standard channels, duty cycle < 1%
    // p2p_channel_cfg(&param[1], EU868_LC1, 0, 7, 1, 16);
    // p2p_channel_cfg(&param[2], EU868_LC2, 0, 7, 1, 16);
    // p2p_channel_cfg(&param[3], EU868_LC3, 0, 7, 1, 16);

    // // 867.1Mhz ~ 867.9Mhz, duty cycle < 1%
    // p2p_channel_cfg(&param[4], 867100000, 0, 7, 1, 16);
    // p2p_channel_cfg(&param[5], 867300000, 0, 7, 1, 16);
    // p2p_channel_cfg(&param[6], 867500000, 0, 7, 1, 16);
    // p2p_channel_cfg(&param[7], 867700000, 0, 7, 1, 16);
    // p2p_channel_cfg(&param[8], 867900000, 0, 7, 1, 16);

    // //Class B channel, duty cycle < 0.1%
    // p2p_channel_cfg(&param[9], 868800000, 0, 7, 1, 16);

    // ping channel, duty cycle < 10%
    p2p_channel_cfg(&param->ping, EU868_PING_SLOT_CHANNEL_FREQ, 1, 6, 1, 16);

    // 3 standard channels, duty cycle < 1%
    p2p_channel_cfg(&param->ch_list[0], EU868_LC1, 1, 6, 1, 16);
    p2p_channel_cfg(&param->ch_list[1], EU868_LC2, 1, 6, 1, 16);
    p2p_channel_cfg(&param->ch_list[2], EU868_LC3, 1, 6, 1, 16);

    // loraWan: 867.1 867.3 ~ 867.9
    // 867.1Mhz ~ 867.9Mhz, duty cycle < 1%
    p2p_channel_cfg(&param->ch_list[3], 866100000, 1, 6, 1, 16);
    p2p_channel_cfg(&param->ch_list[4], 866300000, 1, 6, 1, 16);
    p2p_channel_cfg(&param->ch_list[5], 866500000, 1, 6, 1, 16);
    p2p_channel_cfg(&param->ch_list[6], 866700000, 1, 6, 1, 16);
    p2p_channel_cfg(&param->ch_list[7], 866900000, 1, 6, 1, 16);

    // loraWan: 868.8
    //Class B channel, duty cycle < 0.1%
    p2p_channel_cfg(&param->ch_list[8], 863800000, 1, 6, 1, 16);

    param->list_num = 9;

    for (int i = 0; i < param->list_num; i++) {
        param->bad_list[i] = 0;
    }

    param->uplist = &param->ch_list[0];
    param->uplen = 3;
    param->downlist = &param->ch_list[3];
    param->downlen = param->list_num - 3;
}

void region_eu868_channelstate_reset(p2p_obj_t *obj)
{
    rand_lcg_seed_set(obj->id.auth_key_board);
    for (int i = 0; i < obj->channelgrp.list_num; i++) {
        obj->channelgrp.bad_list[i] = 0;
    }
}

uint8_t region_eu868_downchannelnext(p2p_obj_t *obj, int16_t rssi, int8_t snr)
{
    int cnt_down = 0;
    if (rssi < -95 || snr < -5) {
        obj->channelgrp.bad_list[3+obj->channelgrp.down_freq_idx] = 1;

        P2P_DEBUG("fhss: down channel %d bad\r\n", obj->channelgrp.down_freq_idx);

    }

    do {
        obj->channelgrp.down_freq_idx = rand_lcg_seed_next(obj->channelgrp.downlen);
        if (obj->channelgrp.downlist[obj->channelgrp.down_freq_idx].freq < 800) {

            P2P_DEBUG("invalid freq: %d,%d\r\n", 
                obj->channelgrp.down_freq_idx,
                obj->channelgrp.downlist[obj->channelgrp.down_freq_idx].freq);

        }
        cnt_down++;

        if (cnt_down >= obj->channelgrp.downlen) {
            region_eu868_channelstate_reset(obj);
            obj->channelgrp.down_freq_idx = rand_lcg_seed_next(obj->channelgrp.downlen);
            if (obj->channelgrp.downlist[obj->channelgrp.down_freq_idx].freq < 800) {

                P2P_DEBUG("invalid freq: %d,%d\r\n", 
                    obj->channelgrp.down_freq_idx,
                    obj->channelgrp.downlist[obj->channelgrp.down_freq_idx].freq);

            }

            P2P_DEBUG("fhss: down channel all bad, reset \r\n");

            break;
        }

        if (0 == obj->channelgrp.bad_list[3+obj->channelgrp.down_freq_idx]) {
            break;
        }
    } while(1);

    // obj->channelgrp.down_freq_idx = rand_lcg_seed_next(obj->channelgrp.downlen);
    return 0;
}

uint8_t region_eu868_upchannelnext(p2p_obj_t *obj)
{
    // use fixed up channel
    return 0;
}
