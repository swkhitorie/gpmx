#include "sx126x_hal.h"

#include <drv_gpio.h>
#include <device/spi.h>
struct spi_dev_s *spbus;

void sx126x_dio1_pin_irq(void *arg)
{
    sx126x_irq();
}

void sx126x_hal_init()
{
    // DIO1 Pin
    stm32_gpiosetevent(GET_PINHAL(GPIOF, 0), true, false, true, sx126x_dio1_pin_irq, NULL, 4);

    // RESET Pin
    LOW_INITPIN(GPIOA, 3, IOMODE_OUTPP, IO_PULLUP, IO_SPEEDHIGH);
    sx126x_hal_reset_set(1);

    // BUSY Pin
    LOW_INITPIN(GPIOF, 2, IOMODE_INPUT, IO_NOPULL, IO_SPEEDHIGH);

    spbus = spi_bus_get(1);
}

void sx126x_hal_nss_set(uint8_t state)
{
    // 0:low, 1:high
    LOW_IOSET(GPIOA, 15, state);
}

void sx126x_hal_reset_set(uint8_t state)
{
    // 0:low, 1:high
    LOW_IOSET(GPIOA, 3, state);
}

void sx126x_hal_swctl_1_set(uint8_t state)
{

}

void sx126x_hal_swctl_2_set(uint8_t state)
{

}

void sx126x_hal_wait_onbusy()
{
    while (LOW_IOGET(GPIOF, 2) == 1) {

    }
}

uint8_t sx126x_hal_exchange_byte(uint8_t sndval)
{
    uint8_t sndbyte = sndval;
    uint8_t rcvbyte = 0;
    int ret = 0;

    ret = SPI_EXCHANGEBLOCK(spbus, &sndbyte, &rcvbyte, 1);
    return rcvbyte;
}

