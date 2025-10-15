#ifndef DRIVE_QUAD_SPI_H_
#define DRIVE_QUAD_SPI_H_

#include "drv_cmn.h"
#include "drv_gpio.h"
#include <device/qspi.h>

struct up_qspi_dev_s
{
    struct qspi_dev_s dev;

    uint32_t medium_size;
    bool memmap;
    uint8_t ddr_mode;

    uint8_t id;
    struct periph_pin_t ncspin;
    struct periph_pin_t sckpin;
    struct periph_pin_t io0pin;
    struct periph_pin_t io1pin;
    struct periph_pin_t io2pin;
    struct periph_pin_t io3pin;

    bool mdma_enable;
    uint32_t mdma_priority;
    uint32_t priority;

    volatile uint8_t flag_rx;
    QSPI_HandleTypeDef hqspi;
    MDMA_HandleTypeDef hmdma;
};

extern const struct qspi_ops_s g_qspi_ops;

extern struct qspi_dev_s *g_qspi;

#define STM32_QSPI_MDMA_IRQ(num)  \
    struct up_qspi_dev_s *priv = g_qspi->priv; \
    HAL_MDMA_IRQHandler(priv->hqspi.hmdma);

#endif

