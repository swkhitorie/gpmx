#ifndef P2P_COMMON_H_
#define P2P_COMMON_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "rand_lcg_series.h"
#include "p2p_platform.h"
#include "p2p_config.h"

#include "proto/p2p_proto.h"
#include "proto/p2p_rbuffer.h"

#include "region/region_EU868.h"
#include "region/region_US915.h"
#include "region/region_CN470.h"

#define P2P_VERSION_MAIN   (2)
#define P2P_VERSION_SUB1   (2)
#define P2P_VERSION_SUB2   (0)

#if defined(P2P_REGION_US915)
#define REGION_NVM_MAX_NB_CHANNELS       (US915_MAX_NB_CHANNELS)
#elif defined(P2P_REGION_EU868)
#define REGION_NVM_MAX_NB_CHANNELS       (EU868_MAX_NB_CHANNELS)
#elif defined(P2P_REGION_CN470)
#define REGION_NVM_MAX_NB_CHANNELS       (CN470_MAX_NB_CHANNELS)
#endif

typedef uint16_t (*p2p_forwarding_data)(uint8_t *p, uint16_t len);
typedef uint32_t (*p2p_authkey_generate)(uint32_t *uid, uint32_t key);
typedef void     (*p2p_ant_selection)(uint8_t idx);
typedef void     (*p2p_config_ack)(void *arg);

enum __region {
    LORA_REGION_EU868,           /* European band on 868MHz */
    LORA_REGION_US915,           /* North american band on 915MHz */
    LORA_REGION_CN470,

    LORA_REGION_NUM,
};

enum __p2p_role {
    P2P_SENDER = 0x00,
    P2P_RECEIVER,
};

/**
 * Raw Data Transmission Mode(Fix Direction)
 * Master/Sender    Slave/Receiver
 *        --------------->
 *           Raw Data
 *       <----------------
 *         ACK (Optional)
 * 
 * Config Mode
 * Master/Sender    Slave/Receiver
 *       <----------------
 *          Config Data
 *       ----------------->
 *          Config Ack
 * 
 */
enum __p2p_mode {
    P2P_LISTEN = 0x00,
    P2P_RAW,
    P2P_RAWACK,
    P2P_RAWACK_FHSS,

    P2P_MODE_NUM,
};

enum __p2p_state {
    P2P_LINK_FIND = 0x00,
    P2P_LINK_ESTABLISHED,
    P2P_LINK_CONFIG,
};

enum __p2p_action {
    P2P_ACTION_IDLE = 0x00,
    P2P_ACTION_STATE_TO_CONFIG,
    P2P_ACTION_STATE_TO_CONNECT,
};

#define P2P_CHANNEL_CFG_GET_SF(val) (val & 0x0f)
#define P2P_CHANNEL_CFG_GET_CR(val) ((val & 0xf0) >> 4)

struct __channel_cfg {
    uint32_t freq;   /* frequency */
    uint8_t bw;      /* 0: 125 kHz, 1: 250 kHz, 2: 500 kHz */

    /* BIT[0:3] sf5 ~ sf12 */
    /* BIT[4:7] 1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8 */
    uint8_t sf_cr;
    uint8_t power;   /* 9~22 dbm */
    uint8_t preamblelen; /* 8bits 16bits... */
};

struct __p2p_identity {

    uint32_t uid_board[3];
    uint32_t rand_key_board;
    uint32_t auth_key_board;

    uint32_t uid_obj[3];
    uint32_t rand_key_obj;
    uint32_t auth_key_obj;
};

struct __p2p_status {

    uint32_t seq;
    uint32_t seq_lst;

    uint32_t pack_lost;

    uint32_t nbytes_send;
    uint32_t nbytes_recv;

    uint32_t nbytes_actual_snd;
    uint32_t nbytes_total_snd;
    uint32_t nbytes_total_recv;

    uint32_t ack_timestamp;
    uint32_t ack_completed_time; // unit times: (MaxPack TOA + Get Ack)

    uint32_t ack_wait_timestamp;
    uint32_t ack_wait_time;

    uint32_t lbt_back_time;

    uint32_t rcv_ack_timestamp;
    uint32_t rcv_ack_time;
};

struct __channel_grp {

    struct __channel_cfg current;
    struct __channel_cfg ping;

    struct __channel_cfg ch_list[REGION_NVM_MAX_NB_CHANNELS];
    uint8_t              bad_list[REGION_NVM_MAX_NB_CHANNELS];

    struct __channel_cfg *up_list;
    struct __channel_cfg *dw_list;

    uint8_t              up_ch_len;
    uint8_t              dw_ch_len;
    uint8_t              grp_ch_len;

    uint8_t              dw_fq_idx;
    uint8_t              up_fq_idx;
    uint8_t              dw_fq_idx_fix;
    uint8_t              up_fq_idx_fix;
};

struct __flowctrl_status {
    uint16_t bytes_snd;
    uint16_t bytes_max_ones;
    uint32_t timestamp;
};

struct __p2p_obj {

    enum __region     _region;
    enum __p2p_role   _role;
    enum __p2p_mode   _mode;
    enum __p2p_state  _state;
    enum __p2p_action _action;
    uint8_t           _substate;
    uint8_t           _connect_bond;

    uint8_t                 _max_payload;
    uint8_t                 _cad_idx;

    int16_t                 _dw_ch_rssi;
    int16_t                 _dw_ch_snr;
    int16_t                 _up_ch_rssi;
    int16_t                 _up_ch_snr;

    uint8_t  _ack_fail_retry_cnter;
    uint32_t _ack_fail_timestamp;

    uint32_t _link_find_timestamp;
    uint32_t _link_failed_timestamp;

    struct __p2p_identity   _id_ck;
    struct __p2p_status     _status;

    struct __channel_grp       _ch_grp;

    struct __flowctrl_status   _fctrl;

    p2p_forwarding_data   _fstream;
    p2p_authkey_generate  _fauth_gen;
    p2p_ant_selection     _fant;
    p2p_config_ack        _fack_cfg;

    /**
     * LoRaP2P shared memory
     */
    uint8_t _pbuffer[255];

    struct __prbuf     _prbuf;
    struct __p2p_proto _proto;

    /* Radio Antenna Array Selection */
    uint8_t ant_now;
    uint8_t ant_idx;
    uint8_t ant_test_times;
    uint8_t ant_test_cnter;
    uint8_t ant_idx_best;
    int16_t ant_rssi_best;
    int16_t ant_rssi[P2P_ANTENNA_ARRAY][P2P_ANTENNA_ARRAY_RSSI_LEN];
    bool ant_snder_selection_completed;

    /** platform obj */
    struct p2p_hw hw;

    uint32_t _errcode;
};

#ifdef __cplusplus
extern "C" {
#endif

/************************
 * P2P Running Interface
 ************************/

void p2p_setup(struct __p2p_obj *obj,
    enum __p2p_role role,
    enum __p2p_mode mode,
    enum __region region,
    p2p_forwarding_data fstream,
    p2p_authkey_generate fauth_gen,
    p2p_ant_selection fantsw,
    p2p_config_ack fackcfg,
    uint32_t *uid_board,
    uint32_t uid_key_rng,
    uint16_t fcl_bytes);

void p2p_process(struct __p2p_obj *obj);

void p2p_action(struct __p2p_obj *obj, enum __p2p_action act);

void p2p_channel_grp_reset(struct __p2p_obj *obj);

uint8_t p2p_dw_channel_next(struct __p2p_obj *obj, int16_t rssi, int8_t snr);

uint8_t p2p_up_channel_next(struct __p2p_obj *obj);

/************************
 * SubG Running Mode  
 ************************/
void p2p_state_to_linkfind(struct __p2p_obj *obj);

void p2p_state_to_link_established(struct __p2p_obj *obj);

void p2p_state_to_link_config(struct __p2p_obj *obj);

void p2p_linkfind(struct __p2p_obj *obj);

void p2p_raw_process(struct __p2p_obj *obj);

void p2p_raw_ack_process(struct __p2p_obj *obj);

void p2p_raw_ackfhss_process(struct __p2p_obj *obj);

void p2p_config_process(struct __p2p_obj *obj);

/************************
 * P2P utility Function
 ************************/

void     p2p_channel_set_freq(struct __channel_cfg *ch, uint32_t freq);
uint32_t p2p_channel_get_freq(struct __channel_cfg *ch);
uint8_t  p2p_channel_fq_get_idx(struct __channel_grp *grp, uint32_t freq);
void     p2p_channel_config(struct __channel_cfg *ch, uint32_t freq,
            uint8_t bw, uint8_t sf, uint8_t cr, uint8_t power, uint8_t preamblelen);

void util_id_set_to_uid(uint32_t *dst, uint8_t *src);
void util_id_set_to_array(uint8_t *dst, uint32_t *src);
bool util_id_compare(uint32_t *a, uint8_t *b);
bool util_id_empty(uint32_t *uid);

int  fcl_cal(struct __p2p_obj *obj, int verify_sz);
void fcl_sndbytes(struct __p2p_obj *obj);

void p2p_power_adjust(struct __p2p_obj *obj, int16_t rssi, int8_t snr);

void p2p_antenna_switch_judge(struct __p2p_obj *obj, int16_t rssi);

#ifdef __cplusplus
}
#endif


#endif // P2P_COMMON_H

