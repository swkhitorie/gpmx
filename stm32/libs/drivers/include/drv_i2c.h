#ifndef DRV_I2C_V1_H_
#define DRV_I2C_V1_H_

#include <stdint.h>
#include "drv_common.h"
#include "drv_gpio.h"

#if defined (DRV_BSP_H7)
#include "drv_globalpin_h7.h"
#endif


/* Constants to identify I2C bus's current status */
enum drv_i2c_reg_bus_state {
    I2CSTATE_NULL,         //Not working,not Initialized
    I2CSTATE_START,        //generate a start signal
    I2CSTATE_ADDR,         //Send a Adress Byte to Slave
    I2CSTATE_TXMOD,        //After Send Addr, on Transmit Mode
    I2CSTATE_RXMOD,        //After Send Addr, on Received Mode
    I2CSTATE_TX,           //Sending a Byte to Slave
    I2CSTATE_RX,           //Receiving a Byte From Master
    I2CSTATE_STOP,         //generate a stop signal
    I2CSTATE_FREE,         //current I2C Bus is free
    I2CSTATE_ERROR,        //I2C bus ERROR
};

struct drv_i2c_reg_sensor {
    bool _healthy;
    bool _updated;
    uint64_t _updated_timestamp;
    uint64_t _interval;
};

struct drv_i2c_reg_cmd {
    uint8_t slv_addr;
    uint8_t data_out[5];
    uint8_t data_outlen;
    uint8_t *pdatain;
    uint8_t data_inlen;
    bool istail;
    struct drv_i2c_reg_sensor *pdevice;
};

struct drv_i2c_reg_cmdlist {
    struct drv_i2c_reg_cmd *buf;
    uint8_t capacity;
    uint8_t size;
    uint8_t in;
    uint8_t out;
};

struct drv_i2c_reg_attr_t {
    uint32_t speed;
};

struct drv_i2c_v1_t {
    uint8_t num;
    I2C_HandleTypeDef hi2c;

    uint32_t rcc;
    GPIO_TypeDef scl_port;
    GPIO_TypeDef sda_port;
    uint16_t scl_pin;
    uint16_t sda_pin;

    struct drv_i2c_reg_attr_t attr;
    struct drv_i2c_reg_cmdlist cmdlist;

    struct drv_i2c_reg_cmd current_cmd;
    enum drv_i2c_reg_bus_state state;
    uint8_t cmd_idx;
    uint32_t i2c_direction;
    uint32_t i2c_error_cnt;
};


#ifdef __cplusplus
extern "C" {
#endif

void drv_i2c_reg_attr_config(struct drv_i2c_reg_attr_t *obj, uint32_t speed);

void drv_i2c_reg_cmdlist_config(struct drv_i2c_reg_cmdlist *obj, struct drv_i2c_reg_cmd *buf, uint8_t capacity);

void drv_i2c_v1_config(uint8_t num, struct drv_i2c_v1_t *obj, 
            struct drv_i2c_reg_attr_t *attr, struct drv_i2c_reg_cmdlist *list);

void drv_i2c_v1_init(struct drv_i2c_v1_t *obj);

void drv_i2c_v1_reset(struct drv_i2c_v1_t *obj);


#ifdef cplusplus
}
#endif

#endif
