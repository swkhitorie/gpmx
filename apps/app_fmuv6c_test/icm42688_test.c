#include "icm42688_test.h"
#include <device/spi.h>

struct spi_dev_s *senpi;
void wait_block_1(uint8_t t)
{
    HAL_Delay(t);
}

void icm42688_write_register(uint8_t addr, uint8_t data)
{
    int ret = 0;
	static uint8_t dx[2];
	dx[0] = addr & ~0x80;
	dx[1] = data;

    uint8_t tx_buf[2] = {addr & ~0x80, data};
    uint8_t rx_buf[2] = {0xff, 0xff};

    ret = SPI_SELECT(senpi,0x13,true);
	ret = SPI_EXCHANGE(senpi, &dx[0], NULL, 2);
	//ret = SPI_SNDBLOCK(senpi,&dx[0],2);
    ret = SPI_SELECT(senpi,0x13,false);
}

void icm42688_read_register(uint8_t addr, uint8_t *buf, uint8_t len, int rwway)
{
    int ret = 0;

    if (len != 1) return;

    uint8_t tx_buf[2] = {addr | 0x80, 0x00};
    uint8_t rx_buf[2] = {0};

    ret = SPI_SELECT(senpi,0x13,true);
	ret = SPI_EXCHANGE(senpi, tx_buf, rx_buf, 2);
    ret = SPI_SELECT(senpi,0x13,false);

	buf[0] = rx_buf[1];
}

void icm42688_init()
{
	uint8_t data;

    senpi = dbind("/sensor_spi");
    if (senpi == NULL) {
        printf("not get spi handle");
        return false;
    }

	icm42688_write_register(ICM_426XX_REG_BANK_SEL, 0x00);
	wait_block_1(100);

	icm42688_write_register(ICM_426XX_REG_CHIP_CONFIG, 0x01);
	wait_block_1(100);

	icm42688_write_register(ICM_426XX_REG_BANK_SEL, 0x00);
	wait_block_1(100);

	icm42688_read_register(ICM_426XX_REG_WHO_AM_I, &data, 1, 0);

    if (ICM_426XX_ID != data) {
        printf("[error] The imu id is %02x\n", data);
        return;
    }
	printf("The right imu id is %02x\n", data);
	wait_block_1(10);
	icm42688_write_register(ICM_426XX_REG_DRIVE_CONFIG, 0x05);

	wait_block_1(10);
	icm42688_write_register(ICM_42688_REG_INTF_CONFIG0, 0x10);

    wait_block_1(10);
	icm42688_write_register(ICM_426XX_REG_BANK_SEL, 0x00);

	icm42688_read_register(ICM_42688_REG_PWR_MGMT0, &data, 1, 0);
	data = data | BIT_GYRO_MODE_LN;
    wait_block_1(1);
	icm42688_write_register(ICM_42688_REG_PWR_MGMT0, data);
    wait_block_1(1);

	wait_block_1(50);
	icm42688_read_register(ICM_42688_REG_GYRO_CONFIG0, &data, 1, 0);
	data = (data & 0xF8) | ICM_426XX_GYRO_32KHZ;
	icm42688_write_register(ICM_42688_REG_GYRO_CONFIG0, data);

	wait_block_1(50);
	icm42688_read_register(ICM_42688_REG_GYRO_CONFIG0, &data, 1, 0);
	data = (data & 0x1F) | (ICM_426XX_GYRO_500S << 5);
	icm42688_write_register(ICM_42688_REG_GYRO_CONFIG0, data);
	wait_block_1(50);

	icm42688_write_register(ICM_426XX_REG_BANK_SEL, 0x00);
	icm42688_read_register(ICM_42688_REG_PWR_MGMT0, &data, 1, 0);
    data = data | BIT_ACCEL_MODE_LN;
	icm42688_write_register(ICM_42688_REG_PWR_MGMT0, data);
	wait_block_1(50);

	icm42688_read_register(ICM_42688_REG_ACCEL_CONFIG0, &data, 1, 0);
	data = (data & 0xF8) | ICM_426XX_ACCEL_32KHZ;
    icm42688_write_register(ICM_42688_REG_ACCEL_CONFIG0, data);
	wait_block_1(50);

	icm42688_read_register(ICM_42688_REG_ACCEL_CONFIG0, &data, 1, 0);
	data = (data & 0x1F) | (ICM_426XX_ACCEL_2G << 5);
    icm42688_write_register(ICM_42688_REG_ACCEL_CONFIG0, data);
}

int icm42688_read(int16_t *data)
{
	uint8_t data_sts;
    uint8_t pbuf[6];
    int err = -1;

    icm42688_read_register(ICM_426XX_REG_INT_STATUS, &data_sts, 1, 0);
    if (!(data_sts >> 3)) {
        printf("invalidated data, not ready register-data %02x\n", data);
        return err;
    }

	icm42688_read_register(ICM_426XX_GET_ACCEL_DATA_X0, &pbuf[0], 1, 0);
	icm42688_read_register(ICM_426XX_GET_ACCEL_DATA_X1, &pbuf[1], 1, 0);

	icm42688_read_register(ICM_426XX_GET_ACCEL_DATA_Y0, &pbuf[2], 1, 0);
	icm42688_read_register(ICM_426XX_GET_ACCEL_DATA_Y1, &pbuf[3], 1, 0);

	icm42688_read_register(ICM_426XX_GET_ACCEL_DATA_Z0, &pbuf[4], 1, 0);
	icm42688_read_register(ICM_426XX_GET_ACCEL_DATA_Z1, &pbuf[5], 1, 0);

	data[0] = (signed short)((pbuf[1] << 8) | pbuf[0]);
	data[1] = (signed short)((pbuf[3] << 8) | pbuf[2]);
	data[2] = (signed short)((pbuf[5] << 8) | pbuf[4]);
    return 0;
}
