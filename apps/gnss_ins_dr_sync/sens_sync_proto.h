#ifndef _SENSOR_SYNC_PROTO_H_
#define _SENSOR_SYNC_PROTO_H_

#include <stdint.h>
#include <time.h>

/** 
 * Frame Structure
 * 
 * HEADER(0xFD) MSGID(1byte) LEN(2byte)  PAYLOAD  CHECKSUM(from HEADER ~ PAYLOAD Tail) 
 */

#define SENS_SYNC_PROTO_HEADER          (0xfd)
#define SENS_SYNC_PROTO_PARSEBUFFERLEN    (5*1024)
#define SENS_SYNC_PROTO_PAYLOADIDX      (0x04)

#define SENS_IMU_ID_SCH1633 (0x1001)
#define SENS_IMU_ID_MPU6050 (0x1002)

enum __sens_sync_msg_id {
    SENS_MSGID_ROVER = 0x10,
    SENS_MSGID_BASE  = 0x11,
    SENS_MSGID_WHEELSPEED = 0x20,
    SENS_MSGID_IMU = 0x21,
};

enum __sens_sync_msg_len {

    /** fix msg len */
    SENS_FLEN_IMU = 40,
    SENS_FLEN_WHEELSPEED = 16,
};

struct __sens_sync_proto {
    uint8_t nbyte;  /* bytes idx in msg buffer */ 
    uint16_t len;    /* msg length */
    uint8_t buff[SENS_SYNC_PROTO_PARSEBUFFERLEN];
};

// struct __sens_rover {
//     // raw rtcm3 frame payload
// };

// struct __sens_base {
//     // raw rtcm3 frame payload
// };

struct __sens_sync_imu {
    time_t now;
    uint32_t subsec;
    uint32_t id;
    int32_t accx;     /** 0.001 */
    int32_t accy;     /** 0.001 */
    int32_t accz;     /** 0.001 */
    int32_t gyrox;    /** 0.001 */
    int32_t gyroy;    /** 0.001 */
    int32_t gyroz;    /** 0.001 */
};

struct __sens_sync_wheelspd {
    time_t now;
    uint32_t subsec;
    uint32_t vehicle_speed;
};

#ifdef __cplusplus
extern "C" {
#endif

int sens_sync_proto_parser(struct __sens_sync_proto *proto, uint8_t val);

int sens_sync_encode_rover(struct __sens_sync_proto *proto, const uint8_t *payload, uint16_t len);
int sens_sync_encode_base(struct __sens_sync_proto *proto, const uint8_t *payload, uint16_t len);
int sens_sync_encode_imu(struct __sens_sync_proto *proto, time_t now, uint32_t subsec, uint32_t id, 
    int32_t accx, int32_t accy, int32_t accz, int32_t gyrox, int32_t gyroy, int32_t gyroz);
int sens_sync_encode_wheelspd(struct __sens_sync_proto *proto, time_t now, uint32_t subsec,
    uint32_t speed);

int sens_sync_decode_rover(struct __sens_sync_proto *proto, uint8_t **payload, uint16_t *len);
int sens_sync_decode_base(struct __sens_sync_proto *proto, uint8_t **payload, uint16_t *len);
int sens_sync_decode_imu(struct __sens_sync_proto *proto, time_t *now, uint32_t *subsec, uint32_t *id, 
    int32_t *accx, int32_t *accy, int32_t *accz, int32_t *gyrox, int32_t *gyroy, int32_t *gyroz);
int sens_sync_decode_wheelspd(struct __sens_sync_proto *proto, time_t *now, uint32_t *subsec,
    uint32_t *speed);


#ifdef __cplusplus
}
#endif






#endif
