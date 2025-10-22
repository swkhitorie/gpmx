#ifndef DRV_SPI_H_
#define DRV_SPI_H_

#include "n32g430.h"
#include <drv_gpio.h>

struct n32_spimaster_dev {

    uint8_t id;
    uint8_t mode;
    uint8_t words;

    struct periph_pin_t clkpin;
    struct periph_pin_t mosipin;
    struct periph_pin_t misopin;
    struct periph_pin_t cspin;
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
