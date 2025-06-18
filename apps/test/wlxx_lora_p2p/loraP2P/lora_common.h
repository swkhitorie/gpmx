#ifndef LORA_COMMON_H_
#define LORA_COMMON_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "radio.h"
#include "ringbuffer.h"
#include "rand_lcg_series.h"
#include "lora_p2p_proto.h"

#include "region_EU868.h"
#include "region_US915.h"

/* user include */
#include "board_config.h"

#define LORA_P2P_VERSION_MAIN   (2)
#define LORA_P2P_VERSION_SUB1   (0)
#define LORA_P2P_VERSION_SUB2   (0)

#define LORA_TIMESTAMP_GET()        HAL_GetTick()

/* include wait-connecting message TOA */
#define LORAP2P_WAIT_CONNECTING_TIMEOUT      (200)
#define LORAP2P_ACK_TIMEOUT                  (150)
#define LORAP2P_ACK_TIMEOUT_MAX_TRY_TIMES    (5)
#define LORAP2P_LINK_FAIL_TIMEOUT            (1500)

#define REGION_NVM_MAX_NB_CHANNELS           (72)

#define LORAP2P_DEBUG(...) do {\
    printf("[%d.%03d] ", LORA_TIMESTAMP_GET()/1000, LORA_TIMESTAMP_GET()%1000); \
    if(obj->role == LORA_SENDER) printf("SENDER "); \
    else if(obj->role == LORA_RECEIVER) printf("RECEIVER "); \
    printf(__VA_ARGS__); \
    } while(0)

#define LORA_RADIO_RXBUFF_SZ        (512)

typedef size_t (*forwarding_data)(uint8_t *p, size_t len);

typedef uint32_t (*authkey_generate)(uint32_t *uid, uint32_t key);

typedef enum RADIO_P2P_ROLE {
    LORA_SENDER = 0x00,
    LORA_RECEIVER,
} radio_p2p_role_t;

typedef enum RUNNING_STATE {
    P2P_LINK_FIND = 0x00,
    P2P_LINK_ESTABLISHED,
} radio_p2p_state_t;

typedef uint8_t radio_p2p_substate_t;

typedef enum __lora_region {
    LORA_REGION_EU868,           /* European band on 868MHz */
    LORA_REGION_US915,           /* North american band on 915MHz */
} lora_region_t;

typedef struct __channel_param {

    /* frequency in Hz */
    uint32_t freq;

    /* 0: 125 kHz, 1: 250 kHz, 2: 500 kHz */
    uint8_t bw;

    /* sf5 ~ sf12 */
    uint8_t sf;

    /* 1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8 */
    uint8_t cr; 

    /* 9~22 dbm */
    uint8_t power;

} channel_param_t;

typedef struct __region_grp {

    channel_param_t current;

    channel_param_t ping;

    channel_param_t channels[REGION_NVM_MAX_NB_CHANNELS];

    uint8_t bad_channels[REGION_NVM_MAX_NB_CHANNELS];

    uint8_t channel_num;

} region_grp_t;

typedef struct __lora_state {

    radio_p2p_role_t role;
    lora_region_t region;

    radio_p2p_state_t state;
    radio_p2p_substate_t sub_state;

    uint32_t id_board[3];
    uint32_t id_obj[3];
    uint32_t id_key_board;
    uint32_t id_key_obj;
    uint32_t auth_key_board;
    uint32_t auth_key_obj;

    region_grp_t region_grp;

    uint8_t down_freq_idx;      // sender -> receiver
    uint8_t up_freq_idx;        // receiver -> sender

    uint16_t max_payload;
    uint16_t time_on_air;

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

    int16_t rssi;
    int8_t snr;

    volatile bool tx_done;
    volatile bool rx_crc_error_occur;
    volatile bool rx_timeout_occur;
    volatile bool tx_timeout_occur;
    volatile uint32_t rx_crc_error_cnt;
    volatile uint32_t rx_timeout_cnt;
    volatile uint32_t tx_timeout_cnt;

    uint16_t tick_h;
    uint8_t tick;
    uint8_t tick_lst;

    uint32_t msg_nbytes_send;
    uint32_t msg_nbytes_recv;
    uint32_t msg_nbytes_total_lost;
    uint32_t msg_nbytes_total_snd;
    uint32_t msg_nbytes_total_recv;

    uint16_t crc_error_cnt;         // onRxError() callback triggered times

    uint32_t ack_timeout_timestamp;
    uint16_t ack_timeout_cnter;     // Sender: timeout retry timer counter

    uint32_t link_find_timestamp;   // LinkFind loop times
    uint32_t link_failed_timestamp; // Receiver: check sender online

    uint32_t pack_ack_completed_t1;
    uint32_t pack_ack_completed_time; // Sender: unit times: (one Pack TOA + Get Ack)

} lora_state_t;

#ifdef __cplusplus
extern "C" {
#endif

void util_id_set_to_uid(uint32_t *dst, uint8_t *src);
void util_id_set_to_array(uint8_t *dst, uint32_t *src);
bool util_id_compare(uint32_t *a, uint8_t *b);
bool util_id_empty(uint32_t *uid);

void lora_p2p_objcallback_set(lora_state_t *obj);

void lora_channel_set(channel_param_t *channel, uint32_t freq,
        uint8_t bw, uint8_t sf, uint8_t cr, uint8_t power);

uint32_t lora_p2p_radio_setup(channel_param_t *channel);

uint8_t lora_p2p_downchannelnext(lora_state_t *obj);

uint8_t lora_p2p_upchannelnext(lora_state_t *obj);

void lora_p2p_setup(
    lora_state_t *obj,
    radio_p2p_role_t role,
    lora_region_t region,
    forwarding_data ph,
    authkey_generate auth,
    uint32_t *id,
    uint32_t id_key);

void lora_p2p_process(lora_state_t *obj, uint8_t mode);

void lora_p2p_state_to_linkfind(lora_state_t *obj);

void lora_p2p_state_to_link_established(lora_state_t *obj);

void lora_p2p_linkfind(lora_state_t *obj);

void lora_p2p_raw_process(lora_state_t *obj);

void lora_p2p_raw_ack_process(lora_state_t *obj);

void lora_p2p_raw_ackfhss_process(lora_state_t *obj);

#ifdef __cplusplus
}
#endif


#endif // REGION_COMMON_H

