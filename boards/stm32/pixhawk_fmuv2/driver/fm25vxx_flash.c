#include "fm25vxx_flash.h"
#include <board_config.h>
#include <device/spi.h>

#ifndef TEST_PRINTF
#define TEST_PRINTF    BOARD_PRINTF
#endif

static struct __fm25flash_h {
    struct spi_dev_s *bus;
} _fm25v;
static struct __fm25flash_h *fm25flash_handle_get()
{
    return &_fm25v;
}

uint8_t fm25_flash_read_status()
{
    struct __fm25flash_h *handle = fm25flash_handle_get();
    uint8_t txdata[2] = {FM25_RDSR, 0x00};
    uint8_t rxdata[2] = {0x00, 0x00};

	SPI_SETMODE(handle->bus, SPIDEV_MODE0);
	SPI_SETBITS(handle->bus, 8);
    SPI_SELECT(handle->bus, 0x02, true);
    SPI_EXCHANGEBLOCK(handle->bus, txdata, rxdata, 2);
    SPI_SELECT(handle->bus, 0x02, false);

    return rxdata[1];
}

int fm25_flash_write_enable()
{
    struct __fm25flash_h *handle = fm25flash_handle_get();
    uint8_t cmd = FM25_WRDI;
    uint8_t status = 0;

	SPI_SETMODE(handle->bus, SPIDEV_MODE0);
	SPI_SETBITS(handle->bus, 8);
    SPI_SELECT(handle->bus, 0x02, true);
    SPI_EXCHANGEBLOCK(handle->bus, &cmd, NULL, 1);
    SPI_SELECT(handle->bus, 0x02, false);

    cmd = FM25_WREN;
    SPI_SELECT(handle->bus, 0x02, true);
    SPI_EXCHANGEBLOCK(handle->bus, &cmd, NULL, 1);
    SPI_SELECT(handle->bus, 0x02, false);

    status = fm25_flash_read_status();

    if ((status & FM25_STATUS_WEL) == 0x00) {
        return -1;
    }

    return 0;
}

int fm25_flash_readid()
{
    struct __fm25flash_h *handle = fm25flash_handle_get();
    uint8_t txdata[10] = {0};
    uint8_t rxdata[10] = {0};
	txdata[0] = FM25_RDID;

	SPI_SETMODE(handle->bus, SPIDEV_MODE0);
	SPI_SETBITS(handle->bus, 8);
    SPI_SELECT(handle->bus, 0x02, true);
    SPI_EXCHANGEBLOCK(handle->bus, txdata, rxdata, 10);
    SPI_SELECT(handle->bus, 0x02, false);

    if (rxdata[1] == 0x7f && rxdata[2] == 0x7f && rxdata[3] == 0x7f &&
        rxdata[4] == 0x7f && rxdata[5] == 0x7f && rxdata[6] == 0x7f &&
        rxdata[7] == 0xc2 && rxdata[9] == 0x08) {
        return 0;
    }

    return -1;
}

int fm25_flash_init(int spibus)
{
    int ret = 0;
    struct __fm25flash_h *handle = fm25flash_handle_get();
    if (!handle->bus) {
        handle->bus = spi_bus_get(spibus);
    }

    ret = fm25_flash_readid();
    if (ret != 0) {
        return -1;
    }

    return 0;
}

int fm25_flash_write(uint16_t addr, const uint8_t *p, uint32_t sz)
{
    int ret = 0;
    struct __fm25flash_h *handle = fm25flash_handle_get();
    uint8_t cmd = FM25_WRITE;
    ret = fm25_flash_write_enable();
    if (ret != 0) {
        return -1;
    }

	SPI_SETMODE(handle->bus, SPIDEV_MODE0);
	SPI_SETBITS(handle->bus, 8);
    SPI_SELECT(handle->bus, 0x02, true);
    SPI_EXCHANGEBLOCK(handle->bus, &cmd, NULL, 1);
    SPI_EXCHANGEBLOCK(handle->bus, &addr, NULL, 2);
    SPI_EXCHANGEBLOCK(handle->bus, p, NULL, sz);
    SPI_SELECT(handle->bus, 0x02, false);

    return 0;
}

int fm25_flash_read(uint16_t addr, uint8_t *p, uint32_t sz)
{
    int ret = 0;
    struct __fm25flash_h *handle = fm25flash_handle_get();
    uint8_t cmd = FM25_READ;

	SPI_SETMODE(handle->bus, SPIDEV_MODE0);
	SPI_SETBITS(handle->bus, 8);
    SPI_SELECT(handle->bus, 0x02, true);
    SPI_EXCHANGEBLOCK(handle->bus, &cmd, NULL, 1);
    SPI_EXCHANGEBLOCK(handle->bus, &addr, NULL, 2);
    SPI_EXCHANGE(handle->bus, NULL, p, sz);
    SPI_SELECT(handle->bus, 0x02, false);

    return 0;
}

int fm25_flash_fread(uint16_t addr, uint8_t *p, uint32_t sz)
{
    int ret = 0;
    struct __fm25flash_h *handle = fm25flash_handle_get();
    uint8_t cmd = FM25_FSTRD;
    uint8_t dummy_byte = 0x00;

	SPI_SETMODE(handle->bus, SPIDEV_MODE0);
	SPI_SETBITS(handle->bus, 8);
    SPI_SELECT(handle->bus, 0x02, true);
    SPI_EXCHANGEBLOCK(handle->bus, &cmd, NULL, 1);
    SPI_EXCHANGEBLOCK(handle->bus, &addr, NULL, 2);
    SPI_EXCHANGEBLOCK(handle->bus, &dummy_byte, NULL, 1);
    SPI_EXCHANGE(handle->bus, NULL, p, sz);
    SPI_SELECT(handle->bus, 0x02, false);

    return 0;
}

int fm25_flash_test(int argc, char **argv)
{
    int ret = 0;
    uint8_t data[4] = {0xc1, 0xc2, 0xc3, 0xc6};
    uint8_t rdata[4];

    ret = fm25_flash_readid();
    if (ret != 0) {
        TEST_PRINTF("[fm25v01] id read failed\r\n");
    }

    if (!fm25_flash_write(0x0010, data, 4)) {
        fm25_flash_read(0x0010, rdata, 4);
        TEST_PRINTF("[fm25v01]: %x %x %x %x \r\n", rdata[0], rdata[1], rdata[2], rdata[3]);
    }
}
