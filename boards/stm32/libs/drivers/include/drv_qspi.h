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

    struct periph_pin_t ncspin;
    struct periph_pin_t sckpin;
    struct periph_pin_t io0pin;
    struct periph_pin_t io1pin;
    struct periph_pin_t io2pin;
    struct periph_pin_t io3pin;

    QSPI_HandleTypeDef hqspi;
};

#ifdef cplusplus
extern "C" {
#endif

#ifdef cplusplus
}
#endif


#endif

