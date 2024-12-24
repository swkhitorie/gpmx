#ifndef __LLD_I2C_H_
#define __LLD_I2C_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * low level driver for stm32h7 series, base on cubehal library
 * module i2c
*/

#include "lld_h7_global.h"
#include "lld_gpio.h"

typedef enum __lld_i2c_timing {
    LLD_I2C_STANDARDMODE = 0x00D04BFF, //0x00D04BFF 0x00901954
    LLD_I2C_FASTMODE = 0x20B0CCFF,
} lld_i2c_speed_t;

typedef struct __lld_i2c {
    I2C_HandleTypeDef hi2c;
    uint8_t i2cnum;
    uint32_t speed;
    uint8_t state;
    bool is_busytransmit;
    
    lld_gpio_t sda;
    lld_gpio_t scl;  
} lld_i2c_t;

extern lld_i2c_t *mcu_i2c_list[4];

bool lld_i2c_init(lld_i2c_t *obj, uint8_t i2c_num, uint32_t speed, uint8_t scl_selec, uint8_t sda_selec);
void lld_i2c_blockwait(lld_i2c_t *obj, uint16_t slave_addr);
void lld_i2c_irq_cmdcomplete(lld_i2c_t *obj);

// I2C_MEMADD_SIZE_8BIT / I2C_MEMADD_SIZE_16BIT
uint8_t lld_i2c_write_reg(lld_i2c_t *obj, uint8_t slave, uint8_t reg, uint8_t *writeAddr, uint8_t regtype, uint16_t len, lld_rwway way);
uint8_t lld_i2c_read_reg(lld_i2c_t *obj, uint8_t slave, uint8_t reg, uint8_t *readAddr, uint8_t regtype, uint16_t len, lld_rwway way);

uint8_t lld_i2c_write_single(lld_i2c_t *obj, uint16_t slave, uint8_t *pData, uint16_t Size, lld_rwway way);
uint8_t lld_i2c_read_single(lld_i2c_t *obj, uint16_t slave, uint8_t *pData, uint16_t Size, lld_rwway way);


#ifdef __cplusplus
}
#endif

#endif
