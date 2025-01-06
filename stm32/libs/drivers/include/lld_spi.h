#ifndef __LLD_SPI_H_
#define __LLD_SPI_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * low level driver for stm32h7 series, base on cubehal library
 * module spi
*/

#include "lld_h7_global.h"
#include "lld_gpio.h"
    
typedef enum __lld_spi_mode {
    SPI_MODE_0,
    SPI_MODE_1,
    SPI_MODE_2,
    SPI_MODE_3,
} lld_spi_mode;

typedef struct __lld_spi {
    SPI_HandleTypeDef hspi;
    uint32_t prescaler;
    uint8_t spinum;
    lld_spi_mode mode;
    lld_gpio_t cs_pin;
    lld_gpio_t sck_pin;
    lld_gpio_t miso_pin;
    lld_gpio_t mosi_pin;
} lld_spi_t;

/**
    SPI_MODE_0/SPI_MODE_1/SPI_MODE_2/SPI_MODE_3
    SPI_BaudRatePrescaler_2/4/8/16/32/64/128/256
*/
void lld_spi_init(lld_spi_t *obj, uint8_t num, uint32_t prescaler, lld_spi_mode mode,
    uint8_t spi_nss_selec, uint8_t spi_sck_selec, uint8_t spi_miso_selec, uint8_t spi_mosi_selec);
void lld_spi_write(lld_spi_t *obj, const uint8_t *p, uint16_t len, lld_rwway way);
void lld_spi_read(lld_spi_t *obj, uint8_t *p, uint16_t len);
int lld_spi_readwrite(lld_spi_t *obj, uint8_t *ptx, uint8_t *prx, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif
