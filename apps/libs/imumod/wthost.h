#ifndef WT_IMUHOST_H_
#define WT_IMUHOST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct __wt_proto {
	uint8_t head;
	uint8_t msgid;
	uint8_t payload_len;
	uint8_t checksum;
	uint8_t payload[64];
} wthost_proto_t;

__attribute__((weak)) void wthost_send_buf(const uint8_t *p, uint16_t len);

void wthost_finalize_msg_send(wthost_proto_t *frame);
void wthost_pack_accel_send(wthost_proto_t *frame, float *accel, float temper);
void wthost_pack_gyro_send(wthost_proto_t *frame, float *gyro, float temper);
void wthost_pack_euler_send(wthost_proto_t *frame, float *euler, float temper);

#ifdef __cplusplus
}
#endif

#endif
