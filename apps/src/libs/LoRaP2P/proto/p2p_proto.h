#ifndef P2P_PROTO_H_
#define P2P_PROTO_H_

#include <stdint.h>

#define P2P_PROTO_HEADER                 (0xfa)
#define P2P_PROTO_PAYLOAD_IDX            (3)

#define P2P_PROTO_TRUE   (0)
#define P2P_PROTO_FALSE  (1)

enum __p2p_msg_id {
    P2P_REQ_CONNECT = 0x12,
    P2P_REQ_ALLOW = 0x14,
    P2P_CONNECT_ACK = 0x22,
    P2P_CONNECT_DATAHEAD = 0x24,
};

enum __p2p_msg_len {

    /** fix msg len */
    P2P_FLEN_REQ_CONNECT = 16,
    P2P_FLEN_REQ_ALLOW = 34,
    P2P_FLEN_CONNECT_ACK = 11,

    /** variable msg len */
    P2P_FLEN_CONNECT_DATAHEADER = 13,
};

/**
 * LoRaP2P User Frame
 * 
 * [ HEADER(0xFA) MSGID(1byte) LEN(1byte) PAYLOAD CHECKSUM(From HEADER ~ PAYLOAD Tail) ]
 * LEN(1byte)-> Payload Len
 */

struct __p2p_proto {
    uint8_t nbyte;  /* bytes idx in msg buffer */ 
    uint8_t len;    /* msg length */
    uint8_t buff[255];
};

struct __p2p_req_connect {
    uint8_t rcv_id[12];
    uint32_t rcv_key;
};

struct __p2p_req_allow {
    uint8_t rcv_id[12];
    uint8_t snd_id[12];
    uint32_t rcv_key;
    uint32_t snd_key;
    uint8_t dw_fq_idx;
    uint8_t up_fq_idx;
};

struct __p2p_connect_ack {
    int8_t  snr;               // snr from receiver -> down snr
    int16_t rssi;             // rssi from receiver -> down rssi
    uint32_t seq;
    uint32_t snd_auth_key;
};

struct __p2p_connect_dataheader {
    uint32_t seq;
    uint32_t rcv_auth_key;
    int16_t rssi;             // rssi from sender -> up rssi
    int8_t  snr;               // snr from sender -> up snr
    uint8_t dw_fq_idx;      // Sender -> Receiver
    uint8_t up_fq_idx;      // Receiver -> Sender
};

#ifdef __cplusplus
extern "C" {
#endif

int p2p_proto_parser(struct __p2p_proto *proto, uint8_t val);

int decode_req_connect(struct __p2p_proto *proto, struct __p2p_req_connect *msg);
int encode_req_connect(struct __p2p_proto *proto, struct __p2p_req_connect *msg);

int decode_req_allow(struct __p2p_proto *proto, struct __p2p_req_allow *msg);
int encode_req_allow(struct __p2p_proto *proto, struct __p2p_req_allow *msg);

int decode_connect_ack(struct __p2p_proto *proto, struct __p2p_connect_ack *msg);
int encode_connect_ack(struct __p2p_proto *proto, struct __p2p_connect_ack *msg);

/** return payload length */
int decode_connect_data(struct __p2p_proto *proto, struct __p2p_connect_dataheader *msg, uint8_t **payload, uint8_t *len);
int encode_connect_data(struct __p2p_proto *proto, struct __p2p_connect_dataheader *msg, uint8_t *payload, uint8_t len);

#ifdef __cplusplus
}
#endif


#endif

