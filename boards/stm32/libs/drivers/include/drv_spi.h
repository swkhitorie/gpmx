#ifndef DRIVE_SPI_H_
#define DRIVE_SPI_H_

#include <stdint.h>
#include "drv_cmn.h"
#include "drv_gpio.h"
#include "drv_dma.h"
#include <device/serial.h>

#ifndef CONFIG_SPI_ATTACH_CS_NUM
#define CONFIG_SPI_ATTACH_CS_NUM   4
#endif

#define DEVICE_SPI_DMA_TX  (0x01)
#define DEVICE_SPI_DMA_RX  (0x02)

struct cs_pin_t {
    uint32_t id;
    GPIO_TypeDef *port;
    uint16_t pin;
};

struct up_spi_dev_s
{
    struct spi_dev_s dev;   /* Generic UART device */

    uint8_t id;
    struct periph_pin_t ncspin;
    struct periph_pin_t sckpin;
    struct periph_pin_t misopin;
    struct periph_pin_t mosipin;
    struct dma_config   txdma_cfg;
    struct dma_config   rxdma_cfg;
    uint8_t priority;

    const struct cs_pin_t dev_cs[CONFIG_SPI_ATTACH_CS_NUM];

    SPI_HandleTypeDef hspi;
    DMA_HandleTypeDef txdma;
    DMA_HandleTypeDef rxdma;
    uint32_t clock;
    uint32_t actual_freq;
};

extern const struct spi_ops_s g_spi_ops;

extern struct spi_dev_s *g_spi_list[CONFIG_STM32_SPI_NUM];

#define STM32_SPI_DMA_TX_IRQ(num)  \
    struct up_spi_dev_s *priv = g_spi_list[num-1]->priv; \
    HAL_DMA_IRQHandler(priv->hspi.hdmatx)

#define STM32_SPI_DMA_RX_IRQ(num)  \
    struct up_spi_dev_s *priv = g_spi_list[num-1]->priv; \
    HAL_DMA_IRQHandler(priv->hspi.hdmarx)

#endif
