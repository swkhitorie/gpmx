#include "gfunc.h"

float constrain_float(float amt, float low, float high)
{
	if (isnan(amt)) {
		return (low+high)*0.5f;
	}
	return ((amt)<(low)?(low):((amt)>(high)?(high):(amt)));
}

float safeSqrt(float v)
{
	float ret = sqrtf(v);
	if (isnan(ret)) {
		return 0;
	}
	return ret;
}

float invSqrt(float x)
{
    union {
        float    f;
        uint32_t i;
    } conv = { .f = x };
    conv.i  = 0x5f3759df - (conv.i >> 1);
    conv.f *= 1.5F - (x * 0.5F * conv.f * conv.f);
    return conv.f;
}

void quaternion_to_zyx_euler(quaternion *q, float *pitch, float *roll, float *yaw)
{
	*roll = atan2f(2.0f * (q->q0 * q->q1 + q->q2 * q->q3) , 1.0f - 2.0f * (q->q1 * q->q1 + q->q2 * q->q2));
	*pitch = -1.0f * asinf(2.0f * (q->q1 * q->q3 - q->q0 * q->q2));
	*yaw  = atan2f(2.0f * (q->q0 * q->q3 + q->q1 * q->q2) , 1.0f - 2.0f * (q->q2 * q->q2 + q->q3 * q->q3));
}

void zyx_euler_to_quaternion(quaternion *q, float roll, float pitch, float yaw)
{
    float cy = cosf(yaw * 0.5f);
    float sy = sinf(yaw * 0.5f);
    float cp = cosf(pitch * 0.5f);
    float sp = sinf(pitch * 0.5f);
    float cr = cosf(roll * 0.5f);
    float sr = sinf(roll * 0.5f);

    q->q0 = cy * cp * cr + sy * sp * sr;
    q->q1 = cy * cp * sr - sy * sp * cr;
	q->q2 = sy * cp * sr + cy * sp * cr;
    q->q3 = sy * cp * cr - cy * sp * sr;
}

void accel_coarse_alignment(quaternion *q, float ax, float ay, float az)
{
	float pitch_dg = 0.0f;
	float roll_dg = 0.0f;
	pitch_dg = -1.0f * RadtoAng * atanf(ax * invSqrt(ay * ay + az * az));
	roll_dg = RadtoAng * atanf(ay * invSqrt(ax * ax + az * az));
	zyx_euler_to_quaternion(&obj->_q, roll_dg * AngtoRad, pitch_dg * AngtoRad, 0.0f);
}

