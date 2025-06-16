#ifndef LORA_COMMON_H_
#define LORA_COMMON_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "radio.h"
#include "ringbuffer.h"
#include "lora_p2p_proto.h"

/* user include */
#include "board_config.h"

#define LORA_P2P_VERSION_MAIN   (2)
#define LORA_P2P_VERSION_SUB1   (0)
#define LORA_P2P_VERSION_SUB2   (0)

#define LORA_TIMESTAMP_GET()        HAL_GetTick()

/* include wait-connecting message TOA */
#define LORAP2P_WAIT_CONNECTING_TIMEOUT      (200)

#define LORAP2P_DEBUG(...) do {\
    printf("[%d.%03d] ", LORA_TIMESTAMP_GET()/1000, LORA_TIMESTAMP_GET()%1000); \
    if(obj->role == LORA_SENDER) printf("SENDER "); \
    else if(obj->role == LORA_RECEIVER) printf("RECEIVER "); \
    printf(__VA_ARGS__); \
    } while(0)

#define LORA_RADIO_RXBUFF_SZ        (512)

typedef size_t (*forwarding_data)(uint8_t *p, size_t len);

typedef enum RADIO_P2P_ROLE {
    LORA_SENDER = 0x00,
    LORA_RECEIVER,
} radio_p2p_role_t;

typedef enum RUNNING_STATE {
    P2P_LINK_FIND = 0x00,
    P2P_LINK_ESTABLISHED,
} radio_p2p_state_t;

typedef enum __lora_region {
    LORA_REGION_EU868,           /* European band on 868MHz */
    LORA_REGION_US915,           /* North american band on 915MHz */
} lora_region_t;

typedef struct __lora_state {

    radio_p2p_role_t role;
    radio_p2p_state_t state;
    uint8_t sub_state;

    lora_region_t region;

    uint32_t id_board[3];
    uint32_t id_obj[3];
    uint32_t id_key_board;
    uint32_t id_key_obj;

    uint32_t freq;        /* frequency in Hz */
    uint8_t bw;           /* 0: 125 kHz, 1: 250 kHz, 2: 500 kHz */
    uint8_t sf;           /* sf5 ~ sf12 */
    uint8_t cr;           /* 1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8 */
    uint8_t power;

    uint16_t max_payload;

    /** 
     * for Sender: it forward userdata to rf 
     * for Receiver: it forward rf to userdata
     */
    forwarding_data hp;

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

    bool tx_done;
    bool rx_crc_error_occur;
    bool rx_timeout_occur;
    bool tx_timeout_occur;
    uint32_t rx_crc_error_cnt;
    uint32_t rx_timeout_cnt;
    uint32_t tx_timeout_cnt;

    uint32_t link_find_timestamp;   // board timestamp ms

} lora_state_t;

#ifdef __cplusplus
extern "C" {
#endif

void util_id_set_to_uid(uint32_t *dst, uint8_t *src);
void util_id_set_to_array(uint8_t *dst, uint32_t *src);
bool util_id_compare(uint32_t *a, uint8_t *b);

void lora_p2p_objcallback_set(lora_state_t *obj);

void lora_p2p_setup(
    lora_state_t *obj,
    radio_p2p_role_t role,
    lora_region_t region,
    forwarding_data ph,
    uint32_t *id,
    uint32_t id_key);

void lora_p2p_process(lora_state_t *obj, uint8_t mode);

void lora_p2p_state_to_linkfind(lora_state_t *obj);

void lora_p2p_state_to_link_established(lora_state_t *obj);

void lora_p2p_linkfind(lora_state_t *obj);

void lora_p2p_raw_process(lora_state_t *obj);

#ifdef __cplusplus
}
#endif


#endif // REGION_COMMON_H

