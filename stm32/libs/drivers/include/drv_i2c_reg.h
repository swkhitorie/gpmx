#ifndef DRV_I2C_REG_H_
#define DRV_I2C_REG_H_

#include <stdint.h>
#include "drv_common.h"
#include "drv_gpio.h"

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

struct drv_i2c_reg_cmd
{
    uint8_t slv_addr;
    uint8_t data_out[5];
    uint8_t data_outlen;
    uint8_t *pdatain;
    uint8_t data_inlen;
    bool istail;
    struct drv_i2c_reg_sensor *pdevice;
};

struct drv_i2c_reg_attr_t {
    uint32_t speed;
};

struct drv_i2c_reg_t {
    I2C_TypeDef *com;

};





#endif
