#include "p2p_common.h"

static uint8_t radio_rxmem[RADIO_RXBUFF_SZ];
void p2p_setup(p2p_obj_t *obj, p2p_role_t role, p2p_mode_t mode,
    region_t region, forwarding_data ph, authkey_generate auth,
    uint32_t *id, uint32_t id_key, uint16_t fcl_bytes)
{
    obj->role = role;
    obj->p2p_mode = mode;
    obj->flowctrl_bytes_max_ones = fcl_bytes;
    obj->flowctrl_bytes_snd = 0;
    obj->tx_done = true;
    obj->hp = ph;
    obj->hauth = auth;

    /* init p2p radio rx ringbuffer */
    rb_init(&obj->rf_rxbuf, radio_rxmem, RADIO_RXBUFF_SZ);

    /* generate id[12], comkey */
    memcpy(&obj->id.uid_board[0], &id[0], 12);
    obj->id.rand_key_board = id_key;
    obj->id.auth_key_board = (*obj->hauth)(obj->id.uid_board, obj->id.rand_key_board);
    rand_lcg_seed_set(obj->id.rand_key_board);
    memset((uint8_t *)obj->id.uid_obj, 0, 12);
    obj->isbond = false;

    /* init region channel group */
    obj->region = region;
    switch (region) {
    case LORA_REGION_EU868:
        region_eu868_init_default(&obj->channelgrp);
        break;
    case LORA_REGION_US915:
        region_us915_init_default(&obj->channelgrp);
        break;
    }

    /* init p2p lora param */
    if (obj->p2p_mode == P2P_RAW) {
        obj->channelgrp.current.preamblelen = 16;
        obj->channelgrp.current.bw = 1; //250khz
        obj->channelgrp.current.sf = 7; //sf7
        obj->channelgrp.current.cr = 1;            
    } else {
        obj->channelgrp.current.preamblelen = 16;
        obj->channelgrp.current.bw = 1; //250khz
        obj->channelgrp.current.sf = 6; //sf6
        obj->channelgrp.current.cr = 1; 
    }

    obj->channelgrp.current.power = 16;        //power: 18dbm
    obj->channelgrp.current.freq = obj->channelgrp.ping.freq;    //init with ping channel

    /**
     * init CubeHal Radio Layer Callback
     *      Radio Default(Ping) Channel Param
     *      Max PayloadLen TimeOnAir Param
     *      CONNECT_RESULT TimeOnAir Param
     */
    p2p_if_init(obj, &obj->channelgrp.current);

    char str_region[LORA_REGION_NUM][7] = {
        "EU868",
        "US915"
    };
    char str_p2pmode[P2P_MODE_NUM][16] = {
        "LISTEN",
        "P2P_RAW",
        "P2P_RAWACK",
        "P2P_RAWACK_FHSS",
    };

    P2P_DEBUG("\r\n\tLORA P2P Version:v%d.%d.%d, BOARD ID:%x %x %x\r\n\
\tDefault Freq:%d, Power:%d, BW:%d, SF:%d, CR:%d, PreambleLen:%d, TOA(%d):%dms\r\n\
\tKey: %x, Auth Key: %x\r\n\
\tRegion: %s, Mode: %s, FlowCtrl: %d(B/s)\r\n",
        P2P_VERSION_MAIN, P2P_VERSION_SUB1, P2P_VERSION_SUB2,
        obj->id.uid_board[0], obj->id.uid_board[1], obj->id.uid_board[2],
        obj->channelgrp.current.freq,
        obj->channelgrp.current.power,
        obj->channelgrp.current.bw,
        obj->channelgrp.current.sf,
        obj->channelgrp.current.cr,
        obj->channelgrp.current.preamblelen,
        obj->channelgrp.max_payload,
        obj->channelgrp.time_on_air[0],
        obj->id.rand_key_board,
        obj->id.auth_key_board,
        str_region[obj->region],
        str_p2pmode[obj->p2p_mode],
        obj->flowctrl_bytes_max_ones);

    // select first free down and up channel for Role:Sender
    obj->status.first_enter = true;
    if (obj->role == P2P_SENDER) {
        bool isfind = p2p_detect_first_freechannel(obj, 800, 800, 10000);
        if (!isfind) {

            P2P_DEBUG("Can't Find Any FreeChannel\r\n");

            while(1){}
        }
        obj->channelgrp.fup_idx = rand_lcg_seed_next(obj->channelgrp.uplen);

        P2P_DEBUG("downch: [%d]%d, upch: [%d]%d\r\n", 
            obj->channelgrp.fdown_idx,
            obj->channelgrp.downlist[obj->channelgrp.fdown_idx].freq,
            obj->channelgrp.fup_idx,
            obj->channelgrp.uplist[obj->channelgrp.fup_idx].freq);

    }

    if (obj->p2p_mode == P2P_LISTEN) {

        P2P_DEBUG("start to scan channels rssi\r\n");

    } else {
        p2p_state_to_linkfind(obj);
    }
}

void p2p_process(p2p_obj_t *obj)
{
    switch (obj->p2p_mode) {
    case P2P_LISTEN:
        static int idx_scan = 0;
        if (idx_scan >= obj->channelgrp.list_num - 1) {
            idx_scan = 0;
        } else {
            idx_scan++;
        }

        int16_t rssiThrold = -80;
        int freq = obj->channelgrp.ch_list[idx_scan].freq;
        bool isfree = p2p_ischannelfree(obj, freq, 250000, rssiThrold, 5);
        if (!isfree) {
            P2P_DEBUG("channel %d busy, rssi over %d\r\n", freq, rssiThrold);
        }
        return;
    }

    switch(obj->state) {
    case P2P_LINK_FIND:
        p2p_linkfind(obj);
        break;
    case P2P_LINK_ESTABLISHED:
        if (obj->p2p_mode == P2P_RAW) {
            p2p_raw_process(obj);
        } else if (obj->p2p_mode == P2P_RAWACK) {
            p2p_raw_ack_process(obj);
        } else if (obj->p2p_mode == P2P_RAWACK_FHSS) {
            p2p_raw_ackfhss_process(obj);
        }
        break;
    }
}

void p2p_channelstate_reset(p2p_obj_t *obj)
{
    switch (obj->region) {
    case LORA_REGION_EU868:
        region_eu868_channelstate_reset(obj);
        break;
    case LORA_REGION_US915:
        region_us915_channelstate_reset(obj);
        break;
    }

    return;
}

uint8_t p2p_downchannelnext(p2p_obj_t *obj, int16_t rssi, int8_t snr)
{
    switch (obj->region) {
    case LORA_REGION_EU868:
        region_eu868_downchannelnext(obj, rssi, snr);
        break;
    case LORA_REGION_US915:
        region_us915_downchannelnext(obj, rssi, snr);
        break;
    }

    return 0;
}

uint8_t p2p_upchannelnext(p2p_obj_t *obj)
{
    switch (obj->region) {
    case LORA_REGION_EU868:
        region_eu868_upchannelnext(obj);
        break;
    case LORA_REGION_US915:
        region_us915_upchannelnext(obj);
        break;
    }

    return 0;
}

bool p2p_ischannelbusy(p2p_obj_t *obj)
{
    return obj->cad_busy;
}

uint8_t p2p_channel_idx(p2p_obj_t *obj, uint32_t freq)
{
    uint8_t i = 0;
    for (i = 0; i < obj->channelgrp.list_num; i++) {
        if (freq == obj->channelgrp.ch_list[i].freq) {
            return i;
        }
    }
    return 0;
}

void p2p_channel_cfg(channel_cfg_t *channel, uint32_t freq,
        uint8_t bw, uint8_t sf, uint8_t cr, uint8_t power)
{
    channel->freq = freq;
    channel->bw = bw;
    channel->sf = sf;
    channel->cr = cr;
    channel->power = power;
}

/****************************************************************************
 * UTIL 
 ****************************************************************************/
void util_id_set_to_uid(uint32_t *dst, uint8_t *src)
{
    uint8_t *p = (uint8_t *)dst;
    for (int i = 0; i < 12; i++) {
        p[i] = src[i];
    }
}

void util_id_set_to_array(uint8_t *dst, uint32_t *src)
{
    uint8_t *p = (uint8_t *)src;
    for (int i = 0; i < 12; i++) {
        dst[i] = p[i];
    }
}

bool util_id_compare(uint32_t *a, uint8_t *b)
{
    uint8_t *p = (uint8_t *)a;
    for (int i = 0; i < 12; i++) {
        if (p[i] != b[i]) {
            return false;
        }
    }
    return true;
}

bool util_id_empty(uint32_t *uid)
{
    return (uid[0] == 0) && (uid[1] == 0) && (uid[2] == 0);
}

int fcl_cal(p2p_obj_t *obj, int verify_sz)
{
    int wsz = 0;
    int rsz = 0;

    if (obj->flowctrl_bytes_max_ones <= 0) {
        return obj->channelgrp.max_payload;
    }

    if (obj->flowctrl_bytes_snd == 0) {
        obj->flowctrl_bytes_snd = obj->flowctrl_bytes_max_ones;
        obj->status.flowctrl_timestamp = P2P_TIMESTAMP_GET();

        P2P_DEBUG("first resume\r\n");

    }

    if (P2P_ELAPSED_TIME(obj->status.flowctrl_timestamp) > 1000) {
        obj->flowctrl_bytes_snd = obj->flowctrl_bytes_max_ones;
        obj->status.flowctrl_timestamp = P2P_TIMESTAMP_GET();

        P2P_DEBUG("resume bytes, and start cnt\r\n");

    }

    wsz = (obj->flowctrl_bytes_snd > obj->channelgrp.max_payload) ? 
        obj->channelgrp.max_payload : 
        obj->flowctrl_bytes_snd;

    rsz = wsz - verify_sz;
    if (rsz < verify_sz) {

        // P2P_DEBUG("Remain flow can't send rtcm data except heading \r\n");

        return 0;
    } 

    // P2P_DEBUG("debug1 %d %d %d \r\n", rsz, wsz, verify_sz);

    return rsz;
}

void fcl_sndbytes(p2p_obj_t *obj)
{
    if (obj->flowctrl_bytes_max_ones <= 0) {
        return;
    }
    obj->flowctrl_bytes_snd -= obj->status.nbytes_send;
    return;
}

