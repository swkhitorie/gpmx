#ifndef DRV_SPI_H_
#define DRV_SPI_H_

#include "n32g430.h"
#include <drv_gpio.h>

struct n32_spimaster_dev {

    uint8_t id;
    uint8_t mode;
    uint8_t words;
    
    GPIO_Module   *clk_port;
    uint8_t       clk_pin;
    uint32_t      clk_alternate;

    GPIO_Module   *mosi_port;
    uint8_t       mosi_pin;
    uint32_t      mosi_alternate;
    
    GPIO_Module   *miso_port;
    uint8_t       miso_pin;
    uint32_t      miso_alternate;
    
    GPIO_Module   *cs_port;
    uint8_t       cs_pin;
};

#ifdef __cplusplus
extern "C" {
#endif

void n32_spimaster_init(struct n32_spimaster_dev *dev);
    
uint8_t n32_spimaster_exchange_byte(struct n32_spimaster_dev *dev, uint8_t val);


#ifdef __cplusplus
}
#endif



#endif
