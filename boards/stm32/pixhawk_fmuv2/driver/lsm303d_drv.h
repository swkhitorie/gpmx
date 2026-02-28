#ifndef LSM303D_DRIVE_H_
#define LSM303D_DRIVE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int lsm303d_drv_init(int spibus);
int lsm303d_test_accel();
int lsm303d_test_mag();

#ifdef __cplusplus
}
#endif

#endif

