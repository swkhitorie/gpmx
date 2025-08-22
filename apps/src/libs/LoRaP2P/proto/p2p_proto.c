#include "p2p_proto.h"

static uint8_t p2p_proto_checksum(const uint8_t *p, uint8_t len)
{
    uint8_t i = 0;
    uint8_t sum;
    for (i = 0; i < len; i++) {
        sum += p[i];
    }

    return sum;
}

int p2p_proto_decode(struct __p2p_proto *proto)
{
    int ret = 0;
    uint8_t msg_id = proto->buff[1];

    switch (msg_id) {
    case P2P_REQ_CONNECT:    return (proto->len==P2P_FLEN_REQ_CONNECT)?msg_id:-4;
    case P2P_REQ_ALLOW:      return (proto->len==P2P_FLEN_REQ_ALLOW)?msg_id:-4;
    case P2P_CONNECT_ACK:    return (proto->len==P2P_FLEN_CONNECT_ACK)?msg_id:-4;
    case P2P_CONNECT_DATAHEAD: 
        return (proto->len > P2P_FLEN_CONNECT_DATAHEADER)?msg_id:-4;
    }

    return -5;
}

int p2p_proto_parser(struct __p2p_proto *proto, uint8_t val)
{
    /* synchronize frame */
    if (proto->nbyte==0) {
        if (val!=P2P_PROTO_HEADER) return -1;
        proto->buff[proto->nbyte++]=val;
        return -2;
    }

    proto->buff[proto->nbyte++] = val;

    if (proto->nbyte==3) {
        proto->len=proto->buff[2];
    }

    if (proto->nbyte<3||proto->nbyte<proto->len+4) {
        return -2;
    }
    proto->nbyte=0;

    /* check parity */
    if (p2p_proto_checksum(proto->buff, proto->len+3)!=proto->buff[proto->len+3]) {
        return -3;
    }

    return p2p_proto_decode(proto);
}

/****************************************************************************
 * User Frame Encode-Decode
 ****************************************************************************/
int decode_req_connect(struct __p2p_proto *proto, struct __p2p_req_connect *msg)
{
    int i = 0;
    uint8_t *payload = &proto->buff[P2P_PROTO_PAYLOAD_IDX];

    uint32_t *key = (uint32_t *)&payload[12];
    for (i = 0; i < 12; i++) {
        msg->rcv_id[i] = payload[i];
    }

    msg->rcv_key = *key;

    return P2P_PROTO_TRUE;
}

int encode_req_connect(struct __p2p_proto *proto, struct __p2p_req_connect *msg)
{
    int i = 0;
    uint8_t *payload = &proto->buff[P2P_PROTO_PAYLOAD_IDX];

    proto->buff[0] = P2P_PROTO_HEADER;
    proto->buff[1] = P2P_REQ_CONNECT;
    proto->buff[2] = P2P_FLEN_REQ_CONNECT;
    proto->len = proto->buff[2];

    uint8_t *key = (uint8_t *)(&msg->rcv_key);
    for (i = 0; i < 12; i++) {
        payload[i] = msg->rcv_id[i];
    }

    payload[12] = key[0];
    payload[13] = key[1];
    payload[14] = key[2];
    payload[15] = key[3];

    proto->buff[proto->len+3] = p2p_proto_checksum(proto->buff, proto->len+3);

    return P2P_FLEN_REQ_CONNECT + 4;
}

int decode_req_allow(struct __p2p_proto *proto, struct __p2p_req_allow *msg)
{
    int i = 0;
    uint8_t *payload = &proto->buff[P2P_PROTO_PAYLOAD_IDX];

    uint32_t *rcv_key = (uint32_t *)&payload[24];
    uint32_t *snd_key = (uint32_t *)&payload[28];

    for (i = 0; i < 12; i++) {
        msg->rcv_id[i] = payload[i];
    }

    for (i = 0; i < 12; i++) {
        msg->snd_id[i] = payload[12+i];
    }

    msg->rcv_key = *rcv_key;
    msg->snd_key = *snd_key;
    msg->dw_fq_idx = payload[32];
    msg->up_fq_idx = payload[33];

    return P2P_PROTO_TRUE;
}

int encode_req_allow(struct __p2p_proto *proto, struct __p2p_req_allow *msg)
{
    int i = 0;
    uint8_t *payload = &proto->buff[P2P_PROTO_PAYLOAD_IDX];

    proto->buff[0] = P2P_PROTO_HEADER;
    proto->buff[1] = P2P_REQ_ALLOW;
    proto->buff[2] = P2P_FLEN_REQ_ALLOW;
    proto->len = proto->buff[2];

    uint8_t *rcv_key = (uint8_t *)(&msg->rcv_key);
    uint8_t *snd_key = (uint8_t *)(&msg->snd_key);

    for (i = 0; i < 12; i++) {
        payload[i] = msg->rcv_id[i];
    }
    for (i = 0; i < 12; i++) {
        payload[12+i] = msg->snd_id[i];
    }

    payload[24] = rcv_key[0];
    payload[25] = rcv_key[1];
    payload[26] = rcv_key[2];
    payload[27] = rcv_key[3];

    payload[28] = snd_key[0];
    payload[29] = snd_key[1];
    payload[30] = snd_key[2];
    payload[31] = snd_key[3];

    payload[32] = msg->dw_fq_idx;
    payload[33] = msg->up_fq_idx;

    proto->buff[proto->len+3] = p2p_proto_checksum(proto->buff, proto->len+3);

    return P2P_FLEN_REQ_ALLOW + 4;
}

int decode_connect_ack(struct __p2p_proto *proto, struct __p2p_connect_ack *msg)
{
    uint8_t *payload = &proto->buff[P2P_PROTO_PAYLOAD_IDX];

    int16_t *rssi = (int16_t *)&payload[1];
    uint32_t *seq = (uint32_t *)&payload[3];
    uint32_t *snd_auth_key = (uint32_t *)&payload[7];

    msg->snr = payload[0];
    msg->rssi = *rssi;
    msg->seq = *seq;
    msg->snd_auth_key = *snd_auth_key;

    return P2P_PROTO_TRUE;
}

int encode_connect_ack(struct __p2p_proto *proto, struct __p2p_connect_ack *msg)
{
    uint8_t *payload = &proto->buff[P2P_PROTO_PAYLOAD_IDX];
    uint8_t *snd_auth_key = (uint8_t *)(&msg->snd_auth_key);
    uint8_t *seq = (uint8_t *)(&msg->seq);
    uint8_t *rssi = (uint8_t *)(&msg->rssi);

    proto->buff[0] = P2P_PROTO_HEADER;
    proto->buff[1] = P2P_CONNECT_ACK;
    proto->buff[2] = P2P_FLEN_CONNECT_ACK;
    proto->len = proto->buff[2];

    payload[0] = msg->snr;

    payload[1] = rssi[0];
    payload[2] = rssi[1];

    payload[3] = seq[0];
    payload[4] = seq[1];
    payload[5] = seq[2];
    payload[6] = seq[3];

    payload[7] = snd_auth_key[0];
    payload[8] = snd_auth_key[1];
    payload[9] = snd_auth_key[2];
    payload[10] = snd_auth_key[3];

    proto->buff[proto->len+3] = p2p_proto_checksum(proto->buff, proto->len+3);

    return P2P_FLEN_CONNECT_ACK + 4;
}

int decode_connect_data(struct __p2p_proto *proto, struct __p2p_connect_dataheader *msg, uint8_t **payload, uint8_t *len)
{
    uint8_t *msg_payload = &proto->buff[P2P_PROTO_PAYLOAD_IDX];
    uint32_t *seq = (uint32_t *)&msg_payload[0];
    uint32_t *rcv_auth_key = (uint32_t *)&msg_payload[4];
    int16_t *rssi = (int16_t *)&msg_payload[8];

    msg->seq = *seq;
    msg->rcv_auth_key = *rcv_auth_key;
    msg->rssi = *rssi;
    msg->snr = msg_payload[10];
    msg->dw_fq_idx = msg_payload[11];
    msg->up_fq_idx = msg_payload[12];

    *payload = &msg_payload[13];

    *len = proto->len - P2P_FLEN_CONNECT_DATAHEADER;

    return P2P_PROTO_TRUE;
}

int encode_connect_data(struct __p2p_proto *proto, struct __p2p_connect_dataheader *msg, uint8_t *payload, uint8_t len)
{
    uint8_t i = 0;
    uint8_t *msg_payload = &proto->buff[P2P_PROTO_PAYLOAD_IDX];
    uint8_t *seq = (uint8_t *)(&msg->seq);
    uint8_t *rcv_auth_key = (uint8_t *)(&msg->rcv_auth_key);
    uint8_t *rssi = (uint8_t *)(&msg->rssi);

    proto->buff[0] = P2P_PROTO_HEADER;
    proto->buff[1] = P2P_CONNECT_DATAHEAD;
    proto->buff[2] = P2P_FLEN_CONNECT_DATAHEADER+len;
    proto->len = proto->buff[2];

    msg_payload[0] = seq[0];
    msg_payload[1] = seq[1];
    msg_payload[2] = seq[2];
    msg_payload[3] = seq[3];

    msg_payload[4] = rcv_auth_key[0];
    msg_payload[5] = rcv_auth_key[1];
    msg_payload[6] = rcv_auth_key[2];
    msg_payload[7] = rcv_auth_key[3];

    msg_payload[8] = rssi[0];
    msg_payload[9] = rssi[1];

    msg_payload[10] = msg->snr;
    msg_payload[11] = msg->dw_fq_idx;
    msg_payload[12] = msg->up_fq_idx;

    for (i = 0; i < len; i++) {
        msg_payload[13+i] = payload[i];
    }

    proto->buff[proto->len+3] = p2p_proto_checksum(proto->buff, proto->len+3);

    return P2P_FLEN_CONNECT_DATAHEADER+len+4;
}

