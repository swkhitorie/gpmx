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
    *注：信道8-9为特殊应用信道（如Class B信标）

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

    // 867.1Mhz ~ 867.9Mhz, duty cycle < 1%
    p2p_channel_cfg(&param->ch_list[3], 867100000, 1, 6, 1, 16);
    p2p_channel_cfg(&param->ch_list[4], 867300000, 1, 6, 1, 16);
    p2p_channel_cfg(&param->ch_list[5], 867500000, 1, 6, 1, 16);
    p2p_channel_cfg(&param->ch_list[6], 867700000, 1, 6, 1, 16);
    p2p_channel_cfg(&param->ch_list[7], 867900000, 1, 6, 1, 16);

    //Class B channel, duty cycle < 0.1%
    p2p_channel_cfg(&param->ch_list[8], 868800000, 1, 6, 1, 16);

    param->list_num = 9;

    for (int i = 0; i < param->list_num; i++) {
        param->bad_list[i] = 0;
    }
}

uint8_t region_eu868_downchannelnext(p2p_obj_t *obj)
{
    int arraylen = obj->channelgrp.list_num;

    obj->channelgrp.down_freq_idx = rand_lcg_seed_next(arraylen);

    return 0;
}

uint8_t region_eu868_upchannelnext(p2p_obj_t *obj)
{
    return 0;
}
