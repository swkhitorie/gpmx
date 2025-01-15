#ifndef DRV_SPI_H_
#define DRV_SPI_H_

#include <stdint.h>
#include "drv_common.h"
#include "drv_gpio.h"

#if defined (DRV_BSP_H7)
#include "drv_globalpin_h7.h"
#endif

enum SPIMODE {
    SPI_MODE0,
    SPI_MODE1,
    SPI_MODE2,
    SPI_MODE3,
};

struct drv_spi_attr_t {
    uint8_t mode;
};

struct drv_spi_t {
    SPI_HandleTypeDef hspi;
    uint32_t prescaler;

    struct drv_spi_attr_t attr;
};

#ifdef __cplusplus
extern "C" {
#endif

void drv_spi_attr_init(struct drv_spi_attr_t *obj, uint8_t mode);

void drv_spi_init(uint8_t num, uint32_t prescaler, struct drv_spi_t *obj, 
    struct drv_spi_attr_t *attr, uint8_t nss, uint8_t sck, 
    uint8_t miso, uint8_t mosi);

void drv_spi_write(struct drv_spi_t *obj, const uint8_t *p, uint16_t len, enum __drv_rwway way);

void drv_spi_read(struct drv_spi_t *obj, uint8_t *p, uint16_t len);

int drv_spi_readwrite(struct drv_spi_t *obj, uint8_t *ptx, uint8_t *prx, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif
