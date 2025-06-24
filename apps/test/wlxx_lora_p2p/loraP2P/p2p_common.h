#ifndef P2P_COMMON_H_
#define P2P_COMMON_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "radio.h"
#include "ringbuffer.h"
#include "rand_lcg_series.h"
#include "p2p_proto.h"

#include "region_EU868.h"
#include "region_US915.h"

/* user include */
#include "board_config.h"

#define P2P_VERSION_MAIN   (2)
#define P2P_VERSION_SUB1   (1)
#define P2P_VERSION_SUB2   (0)

#define P2P_TIMESTAMP_GET()        HAL_GetTick()

#define P2P_TIME_DELAY(x)          HAL_Delay(x)

#define P2P_ELAPSED_TIME(x)        board_elapsed_tick(x)

/* include wait-connecting message TOA */
#define P2P_WAIT_CONNECTING_TIMEOUT      (200)
#define P2P_ACK_TIMEOUT                  (150)
#define P2P_ACK_TIMEOUT_MAX_TRY_TIMES    (5)
#define P2P_LINK_FAIL_TIMEOUT            (1500)

#define REGION_NVM_MAX_NB_CHANNELS           (72)

#define P2P_DEBUG(...) do {\
    printf("[%d.%03d] ", P2P_TIMESTAMP_GET()/1000, P2P_TIMESTAMP_GET()%1000); \
    if(obj->role == P2P_SENDER) printf("SENDER "); \
    else if(obj->role == P2P_RECEIVER) printf("RECEIVER "); \
    printf(__VA_ARGS__); \
    } while(0)

#define RADIO_RXBUFF_SZ        (512)

typedef size_t (*forwarding_data)(uint8_t *p, size_t len);

typedef uint32_t (*authkey_generate)(uint32_t *uid, uint32_t key);

typedef enum __region {
    LORA_REGION_EU868,           /* European band on 868MHz */
    LORA_REGION_US915,           /* North american band on 915MHz */
} region_t;

typedef enum __p2p_role {
    P2P_SENDER = 0x00,
    P2P_RECEIVER,
} p2p_role_t;

typedef enum __p2p_mode {
    P2P_LISTEN = 0x00,
    P2P_RAW,
    P2P_RAWACK,
    P2P_RAWACK_FHSS,
} p2p_mode_t;

typedef enum __p2p_state {
    P2P_LINK_FIND = 0x00,
    P2P_LINK_ESTABLISHED,
} p2p_state_t;

typedef uint8_t p2p_substate_t;

typedef struct __p2p_auth {

    uint32_t uid_board[3];

    uint32_t rand_key_board;

    uint32_t auth_key_board;

    uint32_t uid_obj[3];

    uint32_t rand_key_obj;

    uint32_t auth_key_obj;

} p2p_id_auth_t;

typedef struct __p2p_status {

    bool first_enter;

    uint32_t seq;

    uint32_t seq_lst;

    uint32_t nbytes_send;

    uint32_t nbytes_recv;

    uint32_t nbytes_total_lost;

    uint32_t nbytes_total_snd;

    uint32_t nbytes_total_recv;

    uint32_t ack_timestamp;

    uint32_t ack_completed_time; // unit times: (MaxPack TOA + Get Ack)

} p2p_status_t;

typedef struct __channel_cfg {
    uint32_t freq;   /* frequency in Hz */
    uint8_t bw;      /* 0: 125 kHz, 1: 250 kHz, 2: 500 kHz */
    uint8_t sf;      /* sf5 ~ sf12 */
    uint8_t cr;      /* 1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8 */
    uint8_t power;   /* 9~22 dbm */
} channel_cfg_t;

typedef struct __channel_grp {
    channel_cfg_t ping;
    channel_cfg_t ch_list[REGION_NVM_MAX_NB_CHANNELS];

    channel_cfg_t current;
    uint8_t bad_list[REGION_NVM_MAX_NB_CHANNELS];
    uint8_t list_num;

    uint8_t down_freq_idx;      // sender -> receiver
    uint8_t up_freq_idx;        // receiver -> sender

    int16_t down_rssi;
    int16_t down_snr;

    int8_t up_rssi;
    int8_t up_snr;

    uint16_t max_payload;
    uint16_t time_on_air;

    uint8_t first_idx;
} channel_grp_t;


typedef struct __p2p_obj {

    region_t region;
    p2p_role_t role;
    p2p_mode_t p2p_mode;

    p2p_state_t state;
    p2p_substate_t sub_state;

    p2p_id_auth_t id;

    channel_grp_t channelgrp;

    p2p_status_t status;

    uint32_t ack_timeout_timestamp;
    uint16_t ack_timeout_cnter;     // Sender: timeout retry timer counter

    uint32_t link_find_timestamp;   // LinkFind loop times
    uint32_t link_failed_timestamp; // Receiver: check sender online

    /** 
     * for Sender: it forward userdata to rf 
     * for Receiver: it forward rf to userdata
     */
    forwarding_data hp;
    authkey_generate hauth;

    /* radio rx ringbuffer */
    ringbuffer_t rf_rxbuf;

    /**
     * radio rx rtcm3 format decode or
     * radio tx rtcm3 format encode
     */ 
    rtcm_t rf_rtcm;

    /* radio rx byte array read from ringbuffer */
    uint8_t rf_read[255];

    volatile bool cad_done;
    volatile bool cad_busy;
    volatile bool tx_done;
    volatile bool rx_crc_error_occur;
    volatile bool rx_timeout_occur;
    volatile bool tx_timeout_occur;
    volatile uint32_t rx_crc_error_cnt;
    volatile uint32_t rx_timeout_cnt;
    volatile uint32_t tx_timeout_cnt;

} p2p_obj_t;

#ifdef __cplusplus
extern "C" {
#endif

bool p2p_is_tx_done(p2p_obj_t *obj);

int p2p_send(p2p_obj_t *obj, uint8_t *buffer, uint8_t size);

int p2p_lbt_send(p2p_obj_t *obj, uint8_t *buffer, uint8_t size, 
    int16_t rssiThresh, uint16_t unitms, uint8_t continonousTimes, uint32_t totalTimes);

int p2p_cad_send(p2p_obj_t *obj, uint8_t *buffer, uint8_t size, 
    uint16_t unitms, uint8_t continonousTimes, uint32_t totalTimes);

void p2p_objcallback_set(p2p_obj_t *obj);

void p2p_setup(p2p_obj_t *obj, p2p_role_t role, p2p_mode_t mode,
    region_t region, forwarding_data ph, authkey_generate auth,
    uint32_t *id, uint32_t id_key);

void p2p_process(p2p_obj_t *obj);

/************************
 * SubG Running Mode  
 ************************/
void p2p_state_to_linkfind(p2p_obj_t *obj);

void p2p_state_to_link_established(p2p_obj_t *obj);

void p2p_linkfind(p2p_obj_t *obj);

void p2p_raw_process(p2p_obj_t *obj);

void p2p_raw_ack_process(p2p_obj_t *obj);

void p2p_raw_ackfhss_process(p2p_obj_t *obj);


/************************
 * P2P module Function 
 ************************/
void util_id_set_to_uid(uint32_t *dst, uint8_t *src);
void util_id_set_to_array(uint8_t *dst, uint32_t *src);
bool util_id_compare(uint32_t *a, uint8_t *b);
bool util_id_empty(uint32_t *uid);

void p2p_start_cad(p2p_obj_t *obj);

bool p2p_is_channel_busy(p2p_obj_t *obj);

bool p2p_detectchannelfree(p2p_obj_t *obj, uint32_t freq, uint32_t timeout);

bool p2p_detect_first_freechannel(p2p_obj_t *obj, uint32_t unitScanTime,
    uint32_t freeContinuousTime, uint32_t totalScanTime);

void p2p_channel_cfg(channel_cfg_t *channel, uint32_t freq,
        uint8_t bw, uint8_t sf, uint8_t cr, uint8_t power);

bool p2p_ischannelfree(p2p_obj_t *obj, uint32_t freq, 
    uint32_t rxBandwidth, int16_t rssiThresh, uint32_t maxCarrierSenseTime);

bool p2p_scan_first_freechannel(p2p_obj_t *obj, int16_t rssiThresh, uint32_t unitScanTime,
    uint32_t freeContinuousTime, uint32_t totalScanTime);

void p2p_radio_cfg(p2p_obj_t *obj, channel_cfg_t *channel);

uint8_t p2p_downchannelnext(p2p_obj_t *obj);

uint8_t p2p_upchannelnext(p2p_obj_t *obj);

#ifdef __cplusplus
}
#endif


#endif // P2P_COMMON_H

