#include "p2p_common.h"

void p2p_setup(struct __p2p_obj *obj,
    enum __p2p_role role,
    enum __p2p_mode mode,
    enum __region region,
    p2p_forwarding_data fstream,
    p2p_authkey_generate fauth_gen,
    uint32_t *uid_board,
    uint32_t uid_key_rng,
    uint16_t fcl_bytes)
{
    obj->_role = role;
    obj->_mode = mode;
    obj->_region = region;
    obj->_fstream = fstream;
    obj->_fauth_gen = fauth_gen;

    obj->hw.tx_done = true;

    obj->_fctrl.bytes_max_ones = fcl_bytes;
    obj->_fctrl.bytes_snd = 0;

    obj->_connect_bond = false;

    prb_reset(&obj->_prbuf);

    /* generate id[12], comkey */
    memset((uint8_t *)obj->_id_ck.uid_obj, 0, 12);
    memcpy(&obj->_id_ck.uid_board[0], &uid_board[0], 12);
    obj->_id_ck.rand_key_board = uid_key_rng;     // true rand number
    obj->_id_ck.auth_key_board = obj->_fauth_gen(obj->_id_ck.uid_board, obj->_id_ck.rand_key_board);
    rand_lcg_seed_set(obj->_id_ck.rand_key_board);

    /* init region channel group */
    char str_region[LORA_REGION_NUM][7] = {
        "EU868",
        "US915",
        "CN470",
    };
    switch (region) {
    case LORA_REGION_EU868:
        region_eu868_init_default(&obj->_ch_grp);
        break;
    case LORA_REGION_US915:
        region_us915_init_default(&obj->_ch_grp);
        break;
    case LORA_REGION_CN470:
        region_cn470_init_default(&obj->_ch_grp);
        break;
    }

    /* init p2p lora param */
    if (obj->_mode == P2P_RAW) {

        // bw:500KHz, sf:7, cr:1, power: 18, preambleLen: 16
        p2p_channel_config(&obj->_ch_grp.current, obj->_ch_grp.ping.freq, 2, 7, 1, 18, 16);
    } else {

        // bw:2500KHz, sf:6, cr:1, power: 18, preambleLen: 16
        p2p_channel_config(&obj->_ch_grp.current, obj->_ch_grp.ping.freq, 1, 6, 1, 18, 16);
    }

    /**
     * init Radio LowLevel Layer Callback
     *      Radio Default(Ping) Channel Param
     *      Max PayloadLen TimeOnAir Param
     *      CONNECT_RESULT TimeOnAir Param
     */
    p2p_platform_setup(obj, 
        obj->_ch_grp.current.freq,
        obj->_ch_grp.current.bw,
        P2P_CHANNEL_CFG_GET_SF(obj->_ch_grp.current.sf_cr),
        P2P_CHANNEL_CFG_GET_CR(obj->_ch_grp.current.sf_cr),
        obj->_ch_grp.current.power,
        obj->_ch_grp.current.preamblelen,
        242);

    char str_p2pmode[P2P_MODE_NUM][16] = {
        "LISTEN",
        "P2P_RAW",
        "P2P_RAWACK",
        "P2P_RAWACK_FHSS",
    };

    p2p_info("\r\n\tLORA P2P Version:v%d.%d.%d, BOARD ID:%x %x %x\r\n\
\tDefault Freq:%d, Power:%d, BW:%d, SF:%d, CR:%d, PreambleLen:%d\r\n\
\tKey: %x, Auth Key: %x\r\n\
\tRegion: %s, Mode: %s, FlowCtrl: %d(B/s)\r\n",
        P2P_VERSION_MAIN, P2P_VERSION_SUB1, P2P_VERSION_SUB2,
        obj->_id_ck.uid_board[0], obj->_id_ck.uid_board[1], obj->_id_ck.uid_board[2],
        obj->_ch_grp.current.freq,
        obj->_ch_grp.current.power,
        obj->_ch_grp.current.bw,
        P2P_CHANNEL_CFG_GET_SF(obj->_ch_grp.current.sf_cr),
        P2P_CHANNEL_CFG_GET_CR(obj->_ch_grp.current.sf_cr),
        obj->_ch_grp.current.preamblelen,
        obj->_id_ck.rand_key_board,
        obj->_id_ck.auth_key_board,
        str_region[obj->_region],
        str_p2pmode[obj->_mode],
        obj->_fctrl.bytes_max_ones);

    // select first free down and up channel for Role:Sender

    if (obj->_role == P2P_SENDER) {
        bool isfind = p2p_cad_scan_first_free_channel(obj, 800, 800, 10000);
        if (!isfind) {
            p2p_info("Can't Find Any FreeChannel\r\n");
            while(1){}
        }
        obj->_ch_grp.up_fq_idx_fix = rand_lcg_seed_next(obj->_ch_grp.up_ch_len);
        p2p_info("downch: [%d]%d, upch: [%d]%d\r\n", 
            obj->_ch_grp.dw_fq_idx_fix,
            obj->_ch_grp.dw_list[obj->_ch_grp.dw_fq_idx_fix].freq,
            obj->_ch_grp.up_fq_idx_fix,
            obj->_ch_grp.up_list[obj->_ch_grp.up_fq_idx_fix].freq);
    }

    if (obj->_mode == P2P_LISTEN) {
        return;
    }

    p2p_state_to_linkfind(obj);
}

void p2p_process(struct __p2p_obj *obj)
{
    switch(obj->_state) {
    case P2P_LINK_FIND:
        p2p_linkfind(obj);
        break;
    case P2P_LINK_ESTABLISHED:
        if (obj->_mode == P2P_RAW) {
            p2p_raw_process(obj);
        } else if (obj->_mode == P2P_RAWACK) {
            p2p_raw_ack_process(obj);
        } else if (obj->_mode == P2P_RAWACK_FHSS) {
            p2p_raw_ackfhss_process(obj);
        }
        break;
    }
}

void p2p_channel_grp_reset(struct __p2p_obj *obj)
{
    switch (obj->_region) {
    case LORA_REGION_EU868:
        region_eu868_channelstate_reset(&obj->_ch_grp);
        break;
    case LORA_REGION_US915:
        region_us915_channelstate_reset(&obj->_ch_grp);
        break;
    }
}

uint8_t p2p_dw_channel_next(struct __p2p_obj *obj, int16_t rssi, int8_t snr)
{
    switch (obj->_region) {
    case LORA_REGION_EU868:
        region_eu868_downchannelnext(&obj->_ch_grp, rssi, snr);
        break;
    case LORA_REGION_US915:
        region_us915_downchannelnext(&obj->_ch_grp, rssi, snr);
        break;
    }

    return 0;
}

uint8_t p2p_up_channel_next(struct __p2p_obj *obj)
{
    switch (obj->_region) {
    case LORA_REGION_EU868:
        region_eu868_upchannelnext(&obj->_ch_grp);
        break;
    case LORA_REGION_US915:
        region_us915_upchannelnext(&obj->_ch_grp);
        break;
    }

    return 0;
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

uint8_t p2p_channel_fq_get_idx(struct __channel_grp *grp, uint32_t freq)
{
    uint8_t i = 0;
    for (i = 0; i < grp->grp_ch_len; i++) {
        if (freq == grp->ch_list[i].freq) {
            return i;
        }
    }
    return 0;
}

void p2p_channel_config(struct __channel_cfg *ch, uint32_t freq,
            uint8_t bw, uint8_t sf, uint8_t cr, uint8_t power, uint8_t preamblelen)
{
    ch->freq = freq;
    ch->bw = bw;
    ch->sf_cr = (sf & 0x0F) | ((cr << 4) & 0xF0);
    ch->power = power;
    ch->preamblelen = preamblelen;
}

int fcl_cal(struct __p2p_obj *obj, int verify_sz)
{
    int wsz = 0;
    int rsz = 0;

    if (obj->_fctrl.bytes_max_ones <= 0) {
        return obj->_max_payload;
    }

    if (obj->_fctrl.bytes_snd == 0) {
        obj->_fctrl.bytes_snd = obj->_fctrl.bytes_max_ones;
        obj->_fctrl.timestamp = P2P_TIMESTAMP_GET();
        p2p_debug("first resume\r\n");
    }

    if (P2P_ELAPSED_TIME(obj->_fctrl.timestamp) > 1000) {
        obj->_fctrl.bytes_snd = obj->_fctrl.bytes_max_ones;
        obj->_fctrl.timestamp = P2P_TIMESTAMP_GET();
        p2p_debug("resume bytes, and start cnt\r\n");
    }

    wsz = (obj->_fctrl.bytes_snd > obj->_max_payload) ? 
        obj->_max_payload : 
        obj->_fctrl.bytes_snd;

    rsz = wsz - verify_sz;
    if (rsz < verify_sz) {
        p2p_debug("Remain flow can't send rtcm data except heading \r\n");
        return 0;
    }
    p2p_debug("debug1 %d %d %d \r\n", rsz, wsz, verify_sz);

    return rsz;
}

void fcl_sndbytes(struct __p2p_obj *obj)
{
    if (obj->_fctrl.bytes_max_ones <= 0) {
        return;
    }
    obj->_fctrl.bytes_snd -= obj->_status.nbytes_send;
    return;
}

