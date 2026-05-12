#ifndef GENERAL_MATH_FUNCTIONS_H_
#define GENERAL_MATH_FUNCTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#ifndef GRAVITY_MSS
	#define GRAVITY_MSS				9.79135f
#endif
#define RadtoAng                    57.29578f
#define AngtoRad                    0.017453293f

typedef struct __vector3 {
    float x;
    float y;
    float z;
} vector3f;

typedef struct __quaternion {
    float q0;
    float q1;
    float q2;
    float q3;
} quaternion;

float constrain_float(float amt, float low, float high);

float safeSqrt(float v);

float invSqrt(float x);

void quaternion_to_zyx_euler(quaternion *q, float *pitch, float *roll, float *yaw);

void zyx_euler_to_quaternion(quaternion *q, float roll, float pitch, float yaw);

void accel_coarse_alignment(quaternion *q, float ax, float ay, float az);

#ifdef __cplusplus
}
#endif

#endif
