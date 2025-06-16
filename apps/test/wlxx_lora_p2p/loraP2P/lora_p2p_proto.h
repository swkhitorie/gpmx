#ifndef LORA_P2P_PROTO_H_
#define LORA_P2P_PROTO_H_

#include <stdint.h>
#include "rtkcmn.h"

#define RTCM3_REQUEST_CONNECT_ARRAYLEN     (22)
#define RTCM3_REQUEST_ALLOW_ARRAYLEN       (39)

#define RTCM3_CONNECT_VERIFY_ARRAYLEN     (29)
#define RTCM3_CONNECT_RESULT_ARRAYLEN     (30)

typedef struct __req_connect {
    uint8_t typid;            // 0x12
    uint8_t rcv_id[12];
    uint32_t rcv_key;
} req_connect_t;

typedef struct __req_allow {
    uint8_t typid;            // 0x14
    uint8_t rcv_id[12];
    uint8_t snd_id[12];
    uint32_t rcv_key;
    uint32_t snd_key;
} req_allow_t;

typedef struct __connect_verify {
    uint16_t subid;           //(12bits) 666
    uint8_t typid;            //(8bits)  0x24 (sender -> receiver)
    uint32_t freq;            //(20bits) khz EU868: 868000
    uint8_t bw;               //(4bits)  0:125khz, 1:250khz, 2:500khz
    uint8_t sf;               //(4bits)  sf5~sf12 0:sf5 2:sf7
    uint8_t cr;               //(4bits)  1:4/5 2:4/6 3:4/7 4:4/8
    uint8_t txpower;          //(4bits)  9~22dbm 0:9dbm
    uint8_t rcv_id[12];       //(12*8bits)
    uint32_t tick;            //(32bits) sequenct
} connect_verify_t;

typedef struct __connect_result {
    uint16_t subid;           //(12bits) 666
    uint8_t typid;            //(8bits)  0x22 (receiver -> sender)
    uint32_t freq;            //(20bits) khz EU868: 868000
    int16_t rssi;             //(16bits)
    int8_t snr;               //(8bits)
    uint32_t snd_id[12];      //(12*8bits)
    uint32_t tick_ack;        //(32bits) sequenct
} connect_ret_t;

#ifdef __cplusplus
extern "C" {
#endif

int decode_req_connect(rtcm_t *rtcm, req_connect_t *req_connect);
int encode_req_connect(uint8_t *p, req_connect_t *req_connect);

int decode_req_allow(rtcm_t *rtcm, req_allow_t *req_allow);
int encode_req_allow(uint8_t *p, req_allow_t *req_allow);

int decode_connect_verify(rtcm_t *rtcm, uint32_t *freq,
        uint8_t *bw, uint8_t *sf, uint8_t *cr, uint8_t *txpower, 
        uint8_t *rcv_id, uint32_t *tick);
int encode_connect_verify(uint8_t *p, uint32_t freq, 
        uint8_t bw, uint8_t sf, uint8_t cr, uint8_t txpower, 
        uint8_t *rcv_id, uint32_t tick);

int decode_connect_result(rtcm_t *rtcm, uint32_t *freq,
        int16_t *rssi, int8_t *snr, uint32_t *tickack, uint8_t *snd_id);
int encode_connect_result(uint8_t *p, uint32_t freq,
        int16_t rssi, int8_t snr, uint32_t tickack, uint8_t *snd_id);


#ifdef __cplusplus
}
#endif


#endif

