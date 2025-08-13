#ifndef P2P_PROTO_H_
#define P2P_PROTO_H_

#include <stdint.h>
#include "rtkcmn.h"

#define P2P_REQUEST_CONNECT_ARRAYLEN     (23)
#define P2P_REQUEST_ALLOW_ARRAYLEN       (41)

#define P2P_CONNECT_VERIFY_ARRAYLEN     (20)
#define P2P_CONNECT_RESULT_ARRAYLEN     (18)

typedef struct __req_connect {
    uint8_t typid;            // 0x12
    uint8_t rcv_id[12];
    uint32_t rcv_key;
} req_connect_t;

typedef struct __req_allow {
    uint8_t typid;            // 0x14
    uint8_t down_freq_idx;
    uint8_t up_freq_idx;
    uint8_t rcv_id[12];
    uint8_t snd_id[12];
    uint32_t rcv_key;
    uint32_t snd_key;
} req_allow_t;

typedef struct __connect_verify {
    uint8_t typid;            // 0x24
    uint8_t down_freq_idx;    // Sender -> Receiver
    uint8_t up_freq_idx;      // Receiver -> Sender
    int16_t rssi;             // rssi from sender -> up rssi
    int8_t snr;               // snr from sender -> up snr
    uint32_t seq;
    uint32_t rcv_auth_key;
} connect_verify_t;

typedef struct __connect_result {
    uint8_t typid;            // 0x22
    int16_t rssi;             // rssi from receiver -> down rssi
    int8_t snr;               // snr from receiver -> down snr
    uint32_t seq;
    uint32_t snd_auth_key;
} connect_ret_t;

#ifdef __cplusplus
extern "C" {
#endif

int decode_req_connect(rtcm_t *rtcm, req_connect_t *req_connect);
int encode_req_connect(uint8_t *p, req_connect_t *req_connect);

int decode_req_allow(rtcm_t *rtcm, req_allow_t *req_allow);
int encode_req_allow(uint8_t *p, req_allow_t *req_allow);

int decode_connect_verify(rtcm_t *rtcm, connect_verify_t *con_verify);
int encode_connect_verify(uint8_t *p, connect_verify_t *con_verify);

int decode_connect_result(rtcm_t *rtcm, connect_ret_t *con_ret);
int encode_connect_result(uint8_t *p, connect_ret_t *con_ret);

#ifdef __cplusplus
}
#endif


#endif

