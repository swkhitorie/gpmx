#ifndef DRIVE_QUAD_SPI_H_
#define DRIVE_QUAD_SPI_H_

#include "drv_cmn.h"
#include "drv_gpio.h"
#include <device/qspi.h>

struct up_qspi_dev_s
{
    struct qspi_dev_s dev;       /* Externally visible part of the QSPI interface */

    struct periph_pin_t ncspin;
    struct periph_pin_t sckpin;
    struct periph_pin_t io0pin;
    struct periph_pin_t io1pin;
    struct periph_pin_t io2pin;
    struct periph_pin_t io3pin;
    struct dma_config   xferdma_cfg;

    uint32_t base;                /* QSPI controller register base address */
    uint32_t frequency;           /* Requested clock frequency */
    uint32_t actual;              /* Actual clock frequency */
    uint8_t mode;                 /* Mode 0,3 */
    uint8_t nbits;                /* Width of word in bits (8 to 32) */
    uint8_t intf;                 /* QSPI controller number (0) */
    bool initialized;             /* TRUE: Controller has been initialized */
    bool memmap;                  /* TRUE: Controller is in memory mapped mode */

    QSPI_HandleTypeDef hqspi;
    DMA_HandleTypeDef xferdma;
};

#endif

