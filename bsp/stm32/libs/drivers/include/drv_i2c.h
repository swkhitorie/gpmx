#ifndef DRV_I2C_V1_H_
#define DRV_I2C_V1_H_

#include <stdint.h>
#include "drv_common.h"
#include "drv_gpio.h"
#if defined (DRV_BSP_H7)
#include "drv_pin_h7.h"
#endif
#include <dev/i2c_master.h>

struct up_i2c_master_s
{
    struct i2c_master_s dev;

    uint8_t id;
    uint8_t pin_scl;
    uint8_t pin_sda;
    uint8_t priority_event;
    uint8_t priority_error;

    I2C_HandleTypeDef hi2c;
    uint8_t state;
    uint8_t ecnt;
    uint8_t ts_ecnt;
    GPIO_TypeDef *scl_port;
    GPIO_TypeDef *sda_port;
    uint16_t scl_pin;
    uint16_t sda_pin;
};

extern const struct i2c_ops_s g_i2c_master_ops;

#endif
