#include "sens_sync_proto.h"
#include <string.h>

static uint8_t sens_sync_proto_checksum(const uint8_t *p, uint16_t len)
{
    int i = 0;
    uint8_t sum = 0;
    for (i = 0; i < len; i++) {
        sum += p[i];
    }

    return sum;
}

int sens_sync_proto_decode(struct __sens_sync_proto *proto)
{
    int ret = 0;
    uint8_t msg_id = proto->buff[1];

    switch (msg_id) {
    case SENS_MSGID_WHEELSPEED: return (proto->len==SENS_FLEN_WHEELSPEED)?msg_id:-4;
    case SENS_MSGID_IMU:        return (proto->len==SENS_FLEN_IMU)?msg_id:-4;
    case SENS_MSGID_ROVER:
    case SENS_MSGID_BASE: 
        return msg_id;
    }

    return -5;
}

int sens_sync_proto_parser(struct __sens_sync_proto *proto, uint8_t val)
{
    /* synchronize frame */
    if (proto->nbyte==0) {
        if (val!=SENS_SYNC_PROTO_HEADER) return -1;
        proto->buff[proto->nbyte++]=val;
        return -2;
    }

    proto->buff[proto->nbyte++] = val;

    if (proto->nbyte==4) {
        proto->len=proto->buff[2] | (proto->buff[3] << 8);
    }

    if (proto->nbyte<3||proto->nbyte<proto->len+5) {
        return -2;
    }
    proto->nbyte=0;

    /* check parity */
    if (sens_sync_proto_checksum(proto->buff, proto->len+4)!=proto->buff[proto->len+4]) {
        return -3;
    }

    return sens_sync_proto_decode(proto);
}







int sens_sync_encode_rover(struct __sens_sync_proto *proto, const uint8_t *payload, uint16_t len)
{
    int i = 0;
    uint16_t msglen = len;
    uint8_t *msgpayload = &proto->buff[SENS_SYNC_PROTO_PAYLOADIDX];
    uint8_t *plen = (uint8_t *)&msglen;
    proto->len = msglen;

    proto->buff[0] = SENS_SYNC_PROTO_HEADER;
    proto->buff[1] = SENS_MSGID_ROVER;
    proto->buff[2] = plen[0];
    proto->buff[3] = plen[1];

    for (i = 0; i < len; i++) {
        msgpayload[i] = payload[i];
    }

    proto->buff[proto->len+4] = sens_sync_proto_checksum(proto->buff, proto->len+4);

    return msglen+5;
}

int sens_sync_encode_base(struct __sens_sync_proto *proto, const uint8_t *payload, uint16_t len)
{
    int i = 0;
    uint16_t msglen = len;
    uint8_t *msgpayload = &proto->buff[SENS_SYNC_PROTO_PAYLOADIDX];
    uint8_t *plen = (uint8_t *)&msglen;
    proto->len = msglen;

    proto->buff[0] = SENS_SYNC_PROTO_HEADER;
    proto->buff[1] = SENS_MSGID_BASE;
    proto->buff[2] = plen[0];
    proto->buff[3] = plen[1];

    for (i = 0; i < len; i++) {
        msgpayload[i] = payload[i];
    }

    proto->buff[proto->len+4] = sens_sync_proto_checksum(proto->buff, proto->len+4);

    return msglen+5;
}


int sens_sync_encode_imu(struct __sens_sync_proto *proto, time_t now, uint32_t subsec, uint32_t id, 
    int32_t accx, int32_t accy, int32_t accz, int32_t gyrox, int32_t gyroy, int32_t gyroz)
{
    int i = 0;
    uint16_t msglen = 40;
    uint8_t *msgpayload = &proto->buff[SENS_SYNC_PROTO_PAYLOADIDX];
    uint8_t *plen = (uint8_t *)&msglen;
    proto->len = msglen;

    proto->buff[0] = SENS_SYNC_PROTO_HEADER;
    proto->buff[1] = SENS_MSGID_IMU;
    proto->buff[2] = plen[0];
    proto->buff[3] = plen[1];

    memcpy(&msgpayload[0], &now, 8);
    memcpy(&msgpayload[8], &subsec, 4);
    memcpy(&msgpayload[12], &id, 4);
    memcpy(&msgpayload[16], &accx, 4);
    memcpy(&msgpayload[20], &accy, 4);
    memcpy(&msgpayload[24], &accz, 4);
    memcpy(&msgpayload[28], &gyrox, 4);
    memcpy(&msgpayload[32], &gyroy, 4);
    memcpy(&msgpayload[36], &gyroz, 4);

    proto->buff[proto->len+4] = sens_sync_proto_checksum(proto->buff, proto->len+4);

    return msglen+5;
}

int sens_sync_encode_wheelspd(struct __sens_sync_proto *proto, time_t now, uint32_t subsec,
    uint32_t speed)
{
    int i = 0;
    uint16_t msglen = 16;
    uint8_t *msgpayload = &proto->buff[SENS_SYNC_PROTO_PAYLOADIDX];
    uint8_t *plen = (uint8_t *)&msglen;
    proto->len = msglen;

    proto->buff[0] = SENS_SYNC_PROTO_HEADER;
    proto->buff[1] = SENS_MSGID_IMU;
    proto->buff[2] = plen[0];
    proto->buff[3] = plen[1];

    memcpy(&msgpayload[0], &now, 8);
    memcpy(&msgpayload[8], &subsec, 4);
    memcpy(&msgpayload[12], &speed, 4);

    proto->buff[proto->len+4] = sens_sync_proto_checksum(proto->buff, proto->len+4);

    return msglen+5;
}


int sens_sync_decode_rover(struct __sens_sync_proto *proto, uint8_t **payload, uint16_t *len)
{
    uint8_t *msgpayload = &proto->buff[SENS_SYNC_PROTO_PAYLOADIDX];

    *payload = &msgpayload[0];
    *len = proto->len;

    return 0;
}

int sens_sync_decode_base(struct __sens_sync_proto *proto, uint8_t **payload, uint16_t *len)
{
    uint8_t *msgpayload = &proto->buff[SENS_SYNC_PROTO_PAYLOADIDX];

    *payload = &msgpayload[0];
    *len = proto->len;

    return 0;
}

int sens_sync_decode_imu(struct __sens_sync_proto *proto, time_t *now, uint32_t *subsec, uint32_t *id, 
    int32_t *accx, int32_t *accy, int32_t *accz, int32_t *gyrox, int32_t *gyroy, int32_t *gyroz)
{
    uint8_t *msgpayload = &proto->buff[SENS_SYNC_PROTO_PAYLOADIDX];

    *now = *((time_t *)&msgpayload[0]);
    *subsec = *((uint32_t *)&msgpayload[8]);
    *id = *((uint32_t *)&msgpayload[12]);

    *accx = *((int32_t *)&msgpayload[16]);
    *accy = *((int32_t *)&msgpayload[20]);
    *accz = *((int32_t *)&msgpayload[24]);

    *gyrox = *((int32_t *)&msgpayload[28]);
    *gyroy = *((int32_t *)&msgpayload[32]);
    *gyroz = *((int32_t *)&msgpayload[36]);

    return 0;
}

int sens_sync_decode_wheelspd(struct __sens_sync_proto *proto, time_t *now, uint32_t *subsec,
    uint32_t *speed)
{
    uint8_t *msgpayload = &proto->buff[SENS_SYNC_PROTO_PAYLOADIDX];

    *now = *((time_t *)&msgpayload[0]);
    *subsec = *((uint32_t *)&msgpayload[8]);
    *speed = *((uint32_t *)&msgpayload[12]);

    return 0;
}
