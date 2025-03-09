#ifndef DRV_I2C_V1_H_
#define DRV_I2C_V1_H_

#include <stdint.h>
#include "drv_common.h"
#include "drv_gpio.h"

#if defined (DRV_BSP_H7)
#include "drv_pin_h7.h"
#endif

#define DRV_I2C_REG_CMD_DATA_OUT_LEN     (8)

enum drv_i2c_memaddr_size {
    I2CMEMADD_8BITS = I2C_MEMADD_SIZE_8BIT,
    I2CMEMADD_16BITS = I2C_MEMADD_SIZE_16BIT,
};

struct drv_i2c_reg_cmd {
    uint16_t slv_addr;
    uint16_t reg_addr;
    uint16_t reg_addr_type;
    uint8_t data_out[DRV_I2C_REG_CMD_DATA_OUT_LEN];
    uint8_t data_outlen;
    uint8_t *pdatain;
    uint8_t data_inlen;
};

struct drv_i2c_reg_cmdlist {
    struct drv_i2c_reg_cmd *buf;
    uint8_t capacity;
    uint8_t size;
    uint8_t in;
    uint8_t out;
};

struct drv_i2c_attr_t {
    uint32_t speed;
    uint8_t priority_event;
    uint8_t priority_error;
};

struct drv_i2c_t {
    uint8_t num;
    I2C_HandleTypeDef hi2c;

    GPIO_TypeDef *scl_port;
    GPIO_TypeDef *sda_port;
    uint16_t scl_pin;
    uint16_t sda_pin;

    struct drv_i2c_attr_t attr;
    struct drv_i2c_reg_cmdlist cmdlist;

    struct drv_i2c_reg_cmd current_cmd;
    uint8_t state;
    uint32_t i2c_error_cnt;

    bool isbusysend;
};

extern struct drv_i2c_t *drv_i2c_list[4];

#ifdef __cplusplus
extern "C" {
#endif

void drv_i2c_attr_config(struct drv_i2c_attr_t *obj, uint32_t speed, uint8_t pevent, uint32_t perror);

void drv_i2c_cmdlist_config(struct drv_i2c_reg_cmdlist *obj, struct drv_i2c_reg_cmd *buf, uint8_t capacity);

void drv_i2c_config(uint8_t num, uint8_t scls, uint8_t sdas, struct drv_i2c_t *obj, 
            struct drv_i2c_attr_t *attr, struct drv_i2c_reg_cmdlist *list);

void drv_i2c_init(struct drv_i2c_t *obj);

int drv_i2c_reg_write(struct drv_i2c_t *obj, uint16_t slave, uint16_t reg, 
                        uint16_t reg_addr_type, uint8_t *p, uint16_t len, enum __drv_rwway way);

int drv_i2c_reg_read(struct drv_i2c_t *obj, uint16_t slave, uint16_t reg, 
                        uint16_t reg_addr_type, uint8_t *p, uint16_t len, enum __drv_rwway way);

int drv_i2c_transfer(struct drv_i2c_t *obj, uint16_t slave, uint8_t *p, uint16_t len,
                        enum __drv_rwway way);

int drv_i2c_receive(struct drv_i2c_t *obj, uint16_t slave, uint8_t *p, uint16_t len,
                        enum __drv_rwway way);

void drv_i2c_cmd_complete(struct drv_i2c_t *obj);

#ifdef cplusplus
}
#endif

#endif
