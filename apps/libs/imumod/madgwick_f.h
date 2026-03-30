#ifndef MADGWICK_FUSION_H_
#define MADGWICK_FUSION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "gfunc.h"

#define MADGWICK_BETA_MAX           (0.08f)
#define QUAT_DELAY_ARRAY_LEN        (10)

typedef struct __quaternion_d {
	quaternion _q[QUAT_DELAY_ARRAY_LEN];
    uint8_t _ahrs_sync_cnt;
    uint8_t _ahrs_sync_flag;
    uint8_t _ahrs_sync_pos;
} quaternion_delay_t;

/**
 * @param q vehicle iteration quaternion
 * @param qdelay quaternion delay compensation sensor to gyro
 * @param beta madgwick step length
 * @param accel input accel, unit in m/s2 or G
 * @param gyro input gyro, unit in degree
 * @param mag no use
 * @param yaw_ob no use
 * @param dt unit in s
 */
vector3f att_estimate_madgwick(
		quaternion *q, quaternion_delay_t *qdelay, float beta,
		vector3f *accel, vector3f *gyro, vector3f *mag,
		float yaw_ob, float dt);

#ifdef __cplusplus
}
#endif

#endif
