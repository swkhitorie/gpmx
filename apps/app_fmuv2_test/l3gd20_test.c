#include "l3gd20_test.h"
#include <device/spi.h>

struct spi_dev_s *selpi;

void l3gd20_write_register(uint8_t addr, uint8_t data)
{
    int ret = 0;
	static uint8_t dx[2];
	dx[0] = addr & ~0x80;
	dx[1] = data;
    static uint8_t rbuf[2] = {0xff, 0xff};

    ret = SPI_SELECT(selpi,0x11,true);
	//ret = SPI_SNDBLOCK(selpi,&dx[0],2);
    ret = SPI_EXCHANGE(selpi, &dx[0], &rbuf[0], 2);
    ret = SPI_SELECT(selpi,0x11,false);
}

void l3gd20_read_register(uint8_t addr, uint8_t *buf, uint8_t len, int rwway)
{
    int ret = 0;

	static uint8_t send_addr;
    send_addr = addr | 0x80;

    uint8_t tx_buf[2] = {send_addr, 0xFF};
    uint8_t rx_buf[2] = {0};

    ret = SPI_SELECT(selpi,0x11,true);
    ret = SPI_EXCHANGE(selpi, tx_buf, rx_buf, 2);
	// ret = SPI_SNDBLOCK(selpi,&send_addr,1);
	// ret = SPI_RECVBLOCK(selpi,buf,len);
    ret = SPI_SELECT(selpi,0x11,false);
    buf[0] = rx_buf[1];
}


void wait_block_1(uint8_t t)
{
    HAL_Delay(t);
}

void l3gd20_init()
{
    static uint8_t data;

    selpi = dbind("/sensor_spi");
    if (selpi == NULL) {
        printf("not get spi handle");
        return false;
    }

    l3gd20_read_register(ADDR_WHO_AM_I, &data, 1, 0);
    wait_block_1(2);

    l3gd20_read_register(ADDR_WHO_AM_I, &data, 1, 0);
    wait_block_1(2);

    if (data == WHO_I_AM) {
        printf("[l3gd20] id: %x \r\n", data);
    } else {
        printf("[l3gd20] unknown id :%x \r\n", data);
    }
}

int l3gd20_read(int16_t *data)
{

}

