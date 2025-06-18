#include "region_EU868.h"
#include "lora_common.h"

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

void region_eu868_init_default(region_grp_t *param)
{
    // // ping channel, duty cycle < 10%
    // lora_channel_set(&param[0], EU868_PING_SLOT_CHANNEL_FREQ, 0, 7, 1, 16);

    // // 3 standard channels, duty cycle < 1%
    // lora_channel_set(&param[1], EU868_LC1, 0, 7, 1, 16);
    // lora_channel_set(&param[2], EU868_LC2, 0, 7, 1, 16);
    // lora_channel_set(&param[3], EU868_LC3, 0, 7, 1, 16);

    // // 867.1Mhz ~ 867.9Mhz, duty cycle < 1%
    // lora_channel_set(&param[4], 867100000, 0, 7, 1, 16);
    // lora_channel_set(&param[5], 867300000, 0, 7, 1, 16);
    // lora_channel_set(&param[6], 867500000, 0, 7, 1, 16);
    // lora_channel_set(&param[7], 867700000, 0, 7, 1, 16);
    // lora_channel_set(&param[8], 867900000, 0, 7, 1, 16);

    // //Class B channel, duty cycle < 0.1%
    // lora_channel_set(&param[9], 868800000, 0, 7, 1, 16);

    // ping channel, duty cycle < 10%
    lora_channel_set(&param->ping, EU868_PING_SLOT_CHANNEL_FREQ, 1, 6, 1, 16);

    // 3 standard channels, duty cycle < 1%
    lora_channel_set(&param->channels[0], EU868_LC1, 1, 6, 1, 16);
    lora_channel_set(&param->channels[1], EU868_LC2, 1, 6, 1, 16);
    lora_channel_set(&param->channels[2], EU868_LC3, 1, 6, 1, 16);

    // 867.1Mhz ~ 867.9Mhz, duty cycle < 1%
    lora_channel_set(&param->channels[3], 867100000, 1, 6, 1, 16);
    lora_channel_set(&param->channels[4], 867300000, 1, 6, 1, 16);
    lora_channel_set(&param->channels[5], 867500000, 1, 6, 1, 16);
    lora_channel_set(&param->channels[6], 867700000, 1, 6, 1, 16);
    lora_channel_set(&param->channels[7], 867900000, 1, 6, 1, 16);

    //Class B channel, duty cycle < 0.1%
    lora_channel_set(&param->channels[8], 868800000, 1, 6, 1, 16);

    param->channel_num = 9;

    for (int i = 0; i < param->channel_num; i++) {
        param->bad_channels[i] = 0;
    }
}

uint8_t region_eu868_downchannelnext(lora_state_t *obj)
{
    int arraylen = obj->region_grp.channel_num;

    obj->down_freq_idx = rand_lcg_seed_next(arraylen);

    return 0;
}

uint8_t region_eu868_upchannelnext(lora_state_t *obj)
{
    return 0;
}
