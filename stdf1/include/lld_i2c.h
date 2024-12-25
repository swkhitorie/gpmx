#ifndef __LLD_I2C_H_
#define __LLD_I2C_H_

/**
 * low level driver for stm32f1 series, base on std periph library
 * module i2c
*/

#ifdef __cplusplus
extern "C" {
#endif
    
#include "lld_kernel.h"
#include "lld_gpio.h"

#define LLD_I2C_CMD_LIST_SIZE   (25)


typedef struct __lld_i2c_cmd {
    uint8_t saddr;
    uint8_t txbuf[5];
    uint8_t *rxbuf;
    uint8_t txlen;
    uint8_t rxlen;
    bool istail;
    uint64_t recently_completed_timestamp;
} lld_i2c_cmd_t;

typedef struct __lld_i2c_cmdlist {
    lld_i2c_cmd_t buf[LLD_I2C_CMD_LIST_SIZE];
    uint16_t size;
    uint16_t capacity;
    uint16_t idx_in;
    uint16_t idx_out;
} lld_i2c_cmdlist_t;

typedef enum __lld_i2c_state {
    I2C_STATE_NULL = 0x00,  //Not working,not Initialized
    I2C_STATE_START,        //generate a start signal
    I2C_STATE_ADDR,         //Send a Adress Byte to Slave
    I2C_STATE_TXMOD,        //After Send Addr, on Transmit Mode
    I2C_STATE_RXMOD,        //After Send Addr, on Received Mode
    I2C_STATE_TX,           //Sending a Byte to Slave
    I2C_STATE_RX,           //Receiving a Byte From Master
    I2C_STATE_STOP,         //generate a stop signal
    I2C_STATE_FREE,         //current I2C Bus is free
    I2C_STATE_ERROR          //I2C bus ERROR
} lld_i2c_state_t;

typedef struct __lld_i2c {
    I2C_TypeDef *i2c;
    uint8_t i2cnum;
    uint32_t speed;
    uint32_t rcc;
    bool remap;
    
    lld_gpio_t sda;
    lld_gpio_t scl;
    
    lld_i2c_state_t state;
    lld_i2c_cmdlist_t cmdlist;
    lld_i2c_cmd_t currentcmd;
    uint8_t dataidx;
    uint8_t i2c_direction;
    int32_t err_cnt;
    uint8_t reset_cnt;   
} lld_i2c_t;

extern lld_i2c_t* mcu_i2c_list[2];

bool lld_i2c_init(lld_i2c_t *obj, uint8_t i2c_num, uint32_t speed, bool remap);

bool lld_i2c_ishealthy(lld_i2c_t *obj);
bool lld_i2c_isfree(lld_i2c_t *obj);
bool lld_i2c_waitfree(lld_i2c_t *obj);
bool lld_i2c_start(lld_i2c_t *obj);
bool lld_i2c_startnextcmd(lld_i2c_t *obj);
bool lld_i2c_addcmd(lld_i2c_t *obj, uint8_t saddr, uint8_t *txbuf, uint8_t txlen, uint8_t *rxbuf, uint8_t rxlen, bool istail);

void lld_i2c_irq_error(lld_i2c_t *obj);
void lld_i2c_irq_event(lld_i2c_t *obj);

void lld_i2c_cmd_set(lld_i2c_cmd_t *p, uint8_t saddr, uint8_t *txbuf, uint8_t txlen, uint8_t *rxbuf, uint8_t rxlen, bool istail);
void lld_i2c_cmd_copy(const lld_i2c_cmd_t *src, lld_i2c_cmd_t *dst);
uint16_t lld_i2c_cmdlist_size(lld_i2c_cmdlist_t *obj);
void lld_i2c_cmdlist_reset(lld_i2c_cmdlist_t *obj);
bool lld_i2c_cmdlist_read(lld_i2c_cmdlist_t *obj, lld_i2c_cmd_t *p, uint16_t len);
bool lld_i2c_cmdlist_write(lld_i2c_cmdlist_t *obj, const lld_i2c_cmd_t *p, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
