#include "wt_imuhost.h"

void wthost_finalize_msg_send(wthost_proto_t *frame)
{
	uint32_t checksum;
	frame->head = 0x55;
	checksum = frame->head + frame->msgid;
	for (uint8_t i = 0; i < frame->payload_len; i++)
		checksum += frame->payload[i];
	frame->checksum = checksum & 0x00FF;

	wthost_send_buf(&frame->head, 1);
	wthost_send_buf(&frame->msgid, 1);
	wthost_send_buf(&frame->payload[0], frame->payload_len);
	wthost_send_buf(&frame->checksum, 1);
}

void wthost_pack_accel_send(wthost_proto_t *frame, float *accel, float temper)
{
	int16_t temp_raw = (temper - 36.53f) * 340;
	int16_t accel_x = (accel[0] / 9.79135f) * 32767 / 16;
	int16_t accel_y = (accel[1] / 9.79135f) * 32767 / 16;
	int16_t accel_z = (accel[2] / 9.79135f) * 32767 / 16;

	frame->msgid = 0x51;
	frame->payload_len = 8;
	frame->payload[0] = accel_x & 0x00FF;
	frame->payload[1] = (accel_x & 0xFF00) >> 8;
	frame->payload[2] = accel_y & 0x00FF;
	frame->payload[3] = (accel_y & 0xFF00) >> 8;
	frame->payload[4] = accel_z & 0x00FF;
	frame->payload[5] = (accel_z & 0xFF00) >> 8;
	frame->payload[6] = temp_raw & 0x00FF;
	frame->payload[7] = (temp_raw & 0xFF00) >> 8;
	wthost_finalize_msg_send(frame);
}

void wthost_pack_gyro_send(wthost_proto_t *frame, float *gyro, float temper)
{
	int16_t temp_raw = (temper - 36.53f) * 340;
	int16_t gyro_x = (gyro[0]) * 32767 / 2000;
	int16_t gyro_y = (gyro[1]) * 32767 / 2000;
	int16_t gyro_z = (gyro[2]) * 32767 / 2000;

	frame->msgid = 0x52;
	frame->payload_len = 8;
	frame->payload[0] = gyro_x & 0x00FF;
	frame->payload[1] = (gyro_x & 0xFF00) >> 8;
	frame->payload[2] = gyro_y & 0x00FF;
	frame->payload[3] = (gyro_y & 0xFF00) >> 8;
	frame->payload[4] = gyro_z & 0x00FF;
	frame->payload[5] = (gyro_z & 0xFF00) >> 8;
	frame->payload[6] = temp_raw & 0x00FF;
	frame->payload[7] = (temp_raw & 0xFF00) >> 8;
	wthost_finalize_msg_send(frame);
}

void wthost_pack_euler_send(wthost_proto_t *frame, float *euler, float temper)
{
	int16_t temp_raw = (temper - 36.53f) * 340;
	int16_t euler_x = (euler[0]) * 32767 / 180;
	int16_t euler_y = (euler[1]) * 32767 / 180;
	int16_t euler_z = (euler[2]) * 32767 / 180;

	frame->msgid = 0x53;
	frame->payload_len = 8;
	frame->payload[0] = euler_x & 0x00FF;
	frame->payload[1] = (euler_x & 0xFF00) >> 8;
	frame->payload[2] = euler_y & 0x00FF;
	frame->payload[3] = (euler_y & 0xFF00) >> 8;
	frame->payload[4] = euler_z & 0x00FF;
	frame->payload[5] = (euler_z & 0xFF00) >> 8;
	frame->payload[6] = temp_raw & 0x00FF;
	frame->payload[7] = (temp_raw & 0xFF00) >> 8;
	wthost_finalize_msg_send(frame);
}
