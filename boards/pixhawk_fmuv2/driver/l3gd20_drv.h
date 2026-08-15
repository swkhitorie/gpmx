#ifndef L3GD20_DRIVE_H_
#define L3GD20_DRIVE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int l3gd20_drv_init(int spibus);
void l3gd20_test();

#ifdef __cplusplus
}
#endif

#endif

