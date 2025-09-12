#ifndef IMU_ASM330L_H_
#define IMU_ASM330L_H_

#include <stdint.h>

#define REG_WHO_AM_I    (0x0f)
#define ASM330LHH_ID    (0x6b)

#define REG_CTRL1_XL    (0x10)
#define REG_CTRL2_G     (0x11)
#define REG_CTRL3_C     (0x12)
#define REG_CTRL4_C     (0x13)
#define REG_CTRL5_C     (0x14)

#define REG_CTRL6_G     (0x15)
#define REG_CTRL7_G     (0x16)
#define REG_CTRL8_XL    (0x17)
#define REG_CTRL9_XL    (0x18)
#define REG_CTRL10_C    (0x19)

#define REG_OUT_TEMP_L  (0x20)
#define REG_OUT_TEMP_G  (0x21)

#define REG_OUTX_L_G    (0x22)
#define REG_OUTX_H_G    (0x23)
#define REG_OUTY_L_G    (0x24)
#define REG_OUTY_H_G    (0x25)
#define REG_OUTZ_L_G    (0x26)
#define REG_OUTZ_H_G    (0x27)

#define REG_OUTX_L_A    (0x28)
#define REG_OUTX_H_A    (0x29)
#define REG_OUTY_L_A    (0x2A)
#define REG_OUTY_H_A    (0x2B)
#define REG_OUTZ_L_A    (0x2C)
#define REG_OUTZ_H_A    (0x2D)

#ifdef __cplusplus
extern "C" {
#endif

int asm330l_init(int busn);

int asm330l_read(float *data);

#ifdef __cplusplus
}
#endif

#endif
