#include "madgwick_f.h"

vector3f att_estimate_madgwick(
		quaternion *q, quaternion_delay_t *qdelay, float beta,
		vector3f *accel, vector3f *gyro, vector3f *mag,
		float yaw_ob, float dt)
{
	float Normal;
	float recipNorm;
	float s0, s1, s2, s3;
	float qDot1, qDot2, qDot3, qDot4;
	float _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2 ,_8q1, _8q2, q0q0, q1q1, q2q2, q3q3;
	float q0,q1,q2,q3;
	float ax,ay,az;
	float gx,gy,gz;
	float roll,pitch,yaw;

	float beta_cor = 0.0f;
	float spinrate = 0.0f;
	float accel_len = 0.0f;
	float accel_norm_squared = 0.0f;
	float upper_accel_limit = 0.0f;
	float lower_accel_limit = 0.0f;
	vector3f euler;

	ax = accel->x;
	ay = accel->y;
	az = accel->z;
	gx = gyro->x * AngtoRad;
	gy = gyro->y * AngtoRad;
	gz = gyro->z * AngtoRad;

	spinrate = safeSqrt(gx * gx + gy * gy + gz * gz) * RadtoAng;
	accel_len = safeSqrt(ax * ax + ay * ay + az * az) - GRAVITY_MSS;

	accel_norm_squared = ax * ax + ay * ay + az * az;
	upper_accel_limit = GRAVITY_MSS * 1.1f;
	lower_accel_limit = GRAVITY_MSS * 0.9f;

	q0 = q->q0;
	q1 = q->q1;
	q2 = q->q2;
	q3 = q->q3;

	// Rate of change of quaternion from gyroscope
	qDot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
	qDot2 = 0.5f * (q0 * gx + q2 * gz - q3 * gy);
	qDot3 = 0.5f * (q0 * gy - q1 * gz + q3 * gx);
	qDot4 = 0.5f * (q0 * gz + q1 * gy - q2 * gx);

	qdelay->_ahrs_sync_cnt++;
	if (qdelay->_ahrs_sync_cnt >= 2) {
		qdelay->_ahrs_sync_cnt = 0;
		qdelay->_ahrs_sync_flag = 1;

		for (int i = QUAT_DELAY_ARRAY_LEN - 1; i > 0; i--) {
			qdelay->_q[i].q0 = qdelay->_q[i - 1].q0;
			qdelay->_q[i].q1 = qdelay->_q[i - 1].q1;
			qdelay->_q[i].q2 = qdelay->_q[i - 1].q2;
			qdelay->_q[i].q3 = qdelay->_q[i - 1].q3;
		}
	}

	qdelay->_q[0].q0 = q->q0;
	qdelay->_q[0].q1 = q->q1;
	qdelay->_q[0].q2 = q->q2;
	qdelay->_q[0].q3 = q->q3;


	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if( (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) &&
		(accel_norm_squared > lower_accel_limit * lower_accel_limit &&
		accel_norm_squared < upper_accel_limit * upper_accel_limit) &&
        qdelay->_ahrs_sync_flag == 1) {

		qdelay->_ahrs_sync_flag = 0;

		// Normalise accelerometer measurement
		recipNorm = invSqrt(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;

		// Auxiliary variables to avoid repeated arithmetic
		_2q0 = 2.0f * qdelay->_q[qdelay->_ahrs_sync_pos].q0;
		_2q1 = 2.0f * qdelay->_q[qdelay->_ahrs_sync_pos].q1;
		_2q2 = 2.0f * qdelay->_q[qdelay->_ahrs_sync_pos].q2;
		_2q3 = 2.0f * qdelay->_q[qdelay->_ahrs_sync_pos].q3;
		_4q0 = 4.0f * qdelay->_q[qdelay->_ahrs_sync_pos].q0;
		_4q1 = 4.0f * qdelay->_q[qdelay->_ahrs_sync_pos].q1;
		_4q2 = 4.0f * qdelay->_q[qdelay->_ahrs_sync_pos].q2;
		_8q1 = 8.0f * qdelay->_q[qdelay->_ahrs_sync_pos].q1;
		_8q2 = 8.0f * qdelay->_q[qdelay->_ahrs_sync_pos].q2;
		q0q0 = qdelay->_q[qdelay->_ahrs_sync_pos].q0 * qdelay->_q[qdelay->_ahrs_sync_pos].q0;
		q1q1 = qdelay->_q[qdelay->_ahrs_sync_pos].q1 * qdelay->_q[qdelay->_ahrs_sync_pos].q1;
		q2q2 = qdelay->_q[qdelay->_ahrs_sync_pos].q2 * qdelay->_q[qdelay->_ahrs_sync_pos].q2;
		q3q3 = qdelay->_q[qdelay->_ahrs_sync_pos].q3 * qdelay->_q[qdelay->_ahrs_sync_pos].q3;

		// Gradient decent algorithm corrective step
		s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
		s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * qdelay->_q[qdelay->_ahrs_sync_pos].q1 - _2q0 * ay - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
		s2 = 4.0f * q0q0 * qdelay->_q[qdelay->_ahrs_sync_pos].q2 + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
		s3 = 4.0f * q1q1 * qdelay->_q[qdelay->_ahrs_sync_pos].q3 - _2q1 * ax + 4.0f * q2q2 * qdelay->_q[qdelay->_ahrs_sync_pos].q3 - _2q2 * ay;
		recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
		s0 *= recipNorm;
		s1 *= recipNorm;
		s2 *= recipNorm;
		s3 *= recipNorm;

		//beta_cor = beta;
		beta_cor = beta + 0.01f * dt * constrain_float(spinrate, 0.0f, 500.0f);
		beta_cor -= 0.0005f * (accel_len / GRAVITY_MSS);
		beta_cor = constrain_float(beta_cor, beta, MADGWICK_BETA_MAX);

		// Apply feedback step
		qDot1 -= beta_cor * s0;
		qDot2 -= beta_cor * s1;
		qDot3 -= beta_cor * s2;
		qDot4 -= beta_cor * s3;
	}

	// Integrate rate of change of quaternion to yield quaternion
	q->q0 += qDot1 * dt;
	q->q1 += qDot2 * dt;
	q->q2 += qDot3 * dt;
	q->q3 += qDot4 * dt;

	// Normalise quaternion
	Normal = invSqrt(q->q0 * q->q0 + q->q1 * q->q1 + q->q2 * q->q2 + q->q3 * q->q3);
	q->q0 *= Normal;
	q->q1 *= Normal;
	q->q2 *= Normal;
	q->q3 *= Normal;

	quaternion_to_zyx_euler(q, &pitch, &roll, &yaw);
	pitch *= RadtoAng;
	roll *= RadtoAng;
	yaw *= RadtoAng;

	euler.x = roll;
	euler.y = pitch;
	euler.z = yaw;

	return euler;
}
