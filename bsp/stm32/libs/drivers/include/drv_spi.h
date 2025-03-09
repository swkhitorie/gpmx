#ifndef DRV_SPI_H_
#define DRV_SPI_H_

#include <stdint.h>
#include "drv_common.h"
#include "drv_gpio.h"

#if defined (DRV_BSP_H7)
#include "drv_pin_h7.h"
#endif

#define DRV_SPI_PERIPHAL_NUM   CONFIG_SPI_PERIPHAL_NUM

enum SPIMODE {
    SPI_MODE0,
    SPI_MODE1,
    SPI_MODE2,
    SPI_MODE3,
};

enum SPIDATABITS {
    SPI_8BITS = SPI_DATASIZE_8BIT,
    SPI_16BITS = SPI_DATASIZE_16BIT,
};

enum SPISPEED {
    SPI_PRESCAL_2 = SPI_BAUDRATEPRESCALER_2,
    SPI_PRESCAL_4 = SPI_BAUDRATEPRESCALER_4,
    SPI_PRESCAL_8 = SPI_BAUDRATEPRESCALER_8,
    SPI_PRESCAL_16 = SPI_BAUDRATEPRESCALER_16,
    SPI_PRESCAL_32 = SPI_BAUDRATEPRESCALER_32,
    SPI_PRESCAL_64 = SPI_BAUDRATEPRESCALER_64,
    SPI_PRESCAL_128 = SPI_BAUDRATEPRESCALER_128,
    SPI_PRESCAL_256 = SPI_BAUDRATEPRESCALER_256,
};

struct drv_spi_attr_t {
    uint32_t mode;
    uint32_t databits;
    uint32_t speed;
};

struct drv_spi_t {
    SPI_HandleTypeDef hspi;
    uint32_t prescaler;

    struct drv_spi_attr_t attr;
};

extern struct drv_spi_t *drv_spi_list[DRV_SPI_PERIPHAL_NUM];

#ifdef __cplusplus
extern "C" {
#endif

void drv_spi_attr_init(struct drv_spi_attr_t *obj, uint32_t mode, uint32_t databits, uint32_t speed);

void drv_spi_init(uint8_t num, struct drv_spi_t *obj, struct drv_spi_attr_t *attr,
    uint8_t nss, uint8_t sck, uint8_t miso, uint8_t mosi);

int drv_spi_write(struct drv_spi_t *obj, const uint8_t *p, uint16_t len, enum __drv_rwway way);

int drv_spi_read(struct drv_spi_t *obj, uint8_t *p, uint16_t len, enum __drv_rwway way);

int drv_spi_readwrite(struct drv_spi_t *obj, uint8_t *ptx, uint8_t *prx, uint16_t size, enum __drv_rwway way);

#ifdef __cplusplus
}
#endif

#endif
