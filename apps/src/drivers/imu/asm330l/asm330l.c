#include "asm330l.h"

#include <board_config.h>
#include <drv_spi.h>
#include <device/spi.h>

static struct spi_dev_s *bus;

static float acc_scale = 0.0f;
static float gyro_scale = 0.0f;

void write_register(uint8_t addr, uint8_t data)
{
    int ret = 0;
	static uint8_t dx[2];
    static uint8_t rbuf[2] = {0x00, 0x00};

	dx[0] = addr & ~0x80;
	dx[1] = data;

    ret = SPI_SELECT(bus,DRV_MODULE_DEVTYPE_USR,true);
    ret = SPI_EXCHANGEBLOCK(bus, &dx[0], &rbuf[0], 2);
    ret = SPI_SELECT(bus,DRV_MODULE_DEVTYPE_USR,false);
}

uint8_t read_register(uint8_t addr)
{
    int ret = 0;

	static uint8_t send_addr;
    send_addr = addr | 0x80;

    uint8_t tx_buf[2] = {send_addr, 0xFF};
    uint8_t rx_buf[2] = {0};

    ret = SPI_SELECT(bus,DRV_MODULE_DEVTYPE_USR,true);
    ret = SPI_EXCHANGEBLOCK(bus, tx_buf, rx_buf, 2);
    ret = SPI_SELECT(bus,DRV_MODULE_DEVTYPE_USR,false);

    return rx_buf[1];
}

void read_registers(uint8_t addr, uint8_t *buf, uint8_t len)
{
    int ret = 0;

	static uint8_t send_addr;
    send_addr = addr | 0x80;

    uint8_t tx_buf[30] = {send_addr, 0xFF};
    uint8_t rx_buf[30] = {0};

    if (len >= 25) {
        return;
    }

    ret = SPI_SELECT(bus,DRV_MODULE_DEVTYPE_USR,true);
    ret = SPI_EXCHANGEBLOCK(bus, tx_buf, rx_buf, len+1);
    ret = SPI_SELECT(bus,DRV_MODULE_DEVTYPE_USR,false);

    for (int i = 0; i < len; i++) {
        buf[i] = rx_buf[1+i];
    }
}

int asm330l_init(int busn)
{
	int res;
	uint8_t r_value[5] = {0};
	uint8_t w_value[5] = {0x44, 0x6E, 0x6C, (0<<7), (0<<4)};
	uint8_t i;
	uint8_t id;

    bus = spi_bus_get(busn);
    if (!bus) {
        return -1;
    }

	id = read_register(REG_WHO_AM_I);

	if (id != ASM330LHH_ID-1 && id != ASM330LHH_ID) {
		return -2;
	}

	write_register(REG_CTRL3_C,  w_value[0]);	//0x12H enable BDU， enable auto increse
	write_register(REG_CTRL1_XL, w_value[1]);	//0x10H 416Hz, ±8g, lpf enable
	write_register(REG_CTRL2_G,  w_value[2]); //0x11H 416Hz, 2000dps
	write_register(REG_CTRL7_G,  w_value[3]); //0x16H enable G_HM_MODE
	write_register(REG_CTRL6_G,  w_value[4]); //0x15H enable XL_HM_MODE

	r_value[0] = read_register(REG_CTRL3_C);
	r_value[1] = read_register(REG_CTRL1_XL);
	r_value[2] = read_register(REG_CTRL2_G);
	r_value[3] = read_register(REG_CTRL7_G);
	r_value[4] = read_register(REG_CTRL6_G);

	for (i = 0; i < 5; i++) {
		if (r_value[i] != w_value[i]) {
			break;
		}
	}

	if (i != 5) {
		return -3;
	}

	acc_scale = (float)((2 * 8) / 65536.0);
	gyro_scale = (float)((2 * 2000.0) / 65536.0);

	return 0;
}

int asm330l_read(float *rdata)
{
	uint8_t dbuf[12];
	int16_t gy[3], ac[3];
	uint8_t data_addr = REG_OUTX_L_G;

	read_registers(data_addr, &dbuf[0], 12);

	gy[0] = (dbuf[0] | dbuf[1] << 8);
	gy[1] = (dbuf[2] | dbuf[3] << 8);
	gy[2] = (dbuf[4] | dbuf[5] << 8);
	ac[0] = (dbuf[6] | dbuf[7] << 8);
	ac[1] = (dbuf[8] | dbuf[9] << 8);
	ac[2] = (dbuf[10] | dbuf[11] << 8);

	rdata[0] = ac[0] * acc_scale;
	rdata[1] = ac[1] * acc_scale;
	rdata[2] = ac[2] * acc_scale;
	rdata[3] = gy[0] * gyro_scale;
	rdata[4] = gy[1] * gyro_scale;
	rdata[5] = gy[2] * gyro_scale;

	return 0;
}

