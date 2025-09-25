#ifndef DRIVE_I2C_H_
#define DRIVE_I2C_H_

#include <stdint.h>
#include "drv_cmn.h"
#include "drv_gpio.h"
#include <device/i2c_master.h>

struct up_i2c_master_s
{
    struct i2c_master_s dev;

    uint8_t id;
    struct periph_pin_t sclpin;
    struct periph_pin_t sdapin;
    uint8_t priority;
    uint8_t priority_error;

    I2C_HandleTypeDef hi2c;
    uint8_t state;
    uint32_t clock;       // i2c periphal clock freq
    uint32_t frequency;   // i2c actual freq
};

extern const struct i2c_ops_s g_i2c_master_ops;

#endif
