#include "mpu6000_drv.h"
#include <board_config.h>
#include <device/spi.h>

#ifndef TEST_PRINTF
#define TEST_PRINTF    BOARD_PRINTF
#endif

enum MPU_DEVICE_TYPE {
	MPU_DEVICE_TYPE_MPU6000	= 6000,
	MPU_DEVICE_TYPE_ICM20602 = 20602,
	MPU_DEVICE_TYPE_ICM20608 = 20608,
	MPU_DEVICE_TYPE_ICM20689 = 20689
};

#define DIR_READ			0x80
#define DIR_WRITE			0x00

// MPU 6000 registers
#define MPUREG_WHOAMI			0x75
#define MPUREG_SMPLRT_DIV		0x19
#define MPUREG_CONFIG			0x1A
#define MPUREG_GYRO_CONFIG		0x1B
#define MPUREG_ACCEL_CONFIG		0x1C
#define MPUREG_FIFO_EN			0x23
#define MPUREG_INT_PIN_CFG		0x37
#define MPUREG_INT_ENABLE		0x38
#define MPUREG_INT_STATUS		0x3A
#define MPUREG_ACCEL_XOUT_H		0x3B
#define MPUREG_ACCEL_XOUT_L		0x3C
#define MPUREG_ACCEL_YOUT_H		0x3D
#define MPUREG_ACCEL_YOUT_L		0x3E
#define MPUREG_ACCEL_ZOUT_H		0x3F
#define MPUREG_ACCEL_ZOUT_L		0x40
#define MPUREG_TEMP_OUT_H		0x41
#define MPUREG_TEMP_OUT_L		0x42
#define MPUREG_GYRO_XOUT_H		0x43
#define MPUREG_GYRO_XOUT_L		0x44
#define MPUREG_GYRO_YOUT_H		0x45
#define MPUREG_GYRO_YOUT_L		0x46
#define MPUREG_GYRO_ZOUT_H		0x47
#define MPUREG_GYRO_ZOUT_L		0x48
#define MPUREG_USER_CTRL		0x6A
#define MPUREG_PWR_MGMT_1		0x6B
#define MPUREG_PWR_MGMT_2		0x6C
#define MPUREG_FIFO_COUNTH		0x72
#define MPUREG_FIFO_COUNTL		0x73
#define MPUREG_FIFO_R_W			0x74
#define MPUREG_PRODUCT_ID		0x0C
#define MPUREG_TRIM1			0x0D
#define MPUREG_TRIM2			0x0E
#define MPUREG_TRIM3			0x0F
#define MPUREG_TRIM4			0x10
#define MPU_GYRO_DLPF_CFG_256HZ_NOLPF2	0x00  // delay: 0.98ms
#define MPU_GYRO_DLPF_CFG_188HZ	0x01  // delay: 1.9ms
#define MPU_GYRO_DLPF_CFG_98HZ		0x02  // delay: 2.8ms
#define MPU_GYRO_DLPF_CFG_42HZ		0x03  // delay: 4.8ms
#define MPU_GYRO_DLPF_CFG_20HZ		0x04  // delay: 8.3ms
#define MPU_GYRO_DLPF_CFG_10HZ		0x05  // delay: 13.4ms
#define MPU_GYRO_DLPF_CFG_5HZ		0x06  // delay: 18.6ms
#define MPU_GYRO_DLPF_CFG_2100HZ_NOLPF	0x07
#define MPU_DLPF_CFG_MASK		0x07

// Configuration bits MPU 3000 and MPU 6000 (not revised)?
#define BIT_SLEEP				0x40
#define BIT_H_RESET				0x80
#define BITS_CLKSEL				0x07
#define MPU_CLK_SEL_PLLGYROX	0x01
#define MPU_CLK_SEL_PLLGYROZ	0x03
#define MPU_EXT_SYNC_GYROX		0x02
#define BITS_GYRO_ST_X			0x80
#define BITS_GYRO_ST_Y			0x40
#define BITS_GYRO_ST_Z			0x20
#define BITS_FS_250DPS			0x00
#define BITS_FS_500DPS			0x08
#define BITS_FS_1000DPS			0x10
#define BITS_FS_2000DPS			0x18
#define BITS_FS_MASK			0x18
#define BIT_INT_ANYRD_2CLEAR	0x10
#define BIT_RAW_RDY_EN			0x01
#define BIT_I2C_IF_DIS			0x10
#define BIT_INT_STATUS_DATA		0x01

#define MPU_WHOAMI_6000			0x68
#define ICM_WHOAMI_20602		0x12
#define ICM_WHOAMI_20608		0xaf
#define ICM_WHOAMI_20689		0x98

// ICM2608 specific registers

#define ICMREG_ACCEL_CONFIG2		0x1D
#define ICM_ACC_DLPF_CFG_1046HZ_NOLPF	0x00
#define ICM_ACC_DLPF_CFG_218HZ		0x01
#define ICM_ACC_DLPF_CFG_99HZ		0x02
#define ICM_ACC_DLPF_CFG_44HZ		0x03
#define ICM_ACC_DLPF_CFG_21HZ		0x04
#define ICM_ACC_DLPF_CFG_10HZ		0x05
#define ICM_ACC_DLPF_CFG_5HZ		0x06
#define ICM_ACC_DLPF_CFG_420HZ		0x07
/* this is an undocumented register which
   if set incorrectly results in getting a 2.7m/s/s offset
   on the Y axis of the accelerometer
*/
#define MPUREG_ICM_UNDOC1		0x11
#define MPUREG_ICM_UNDOC1_VALUE	0xc9

// Product ID Description for ICM20602
// Read From device

#define ICM20602_REV_01		1
#define ICM20602_REV_02		2

// Product ID Description for ICM20608

#define ICM20608_REV_FF		0xff // In the past, was thought to be not returning a value. But seem repeatable.

// Product ID Description for ICM20689

#define ICM20689_REV_FE		0xfe
#define ICM20689_REV_03   0x03
#define ICM20689_REV_04   0x04

// Product ID Description for MPU6000
// high 4 bits 	low 4 bits
// Product Name	Product Revision
#define MPU6000ES_REV_C4		0x14
#define MPU6000ES_REV_C5		0x15
#define MPU6000ES_REV_D6		0x16
#define MPU6000ES_REV_D7		0x17
#define MPU6000ES_REV_D8		0x18
#define MPU6000_REV_C4			0x54
#define MPU6000_REV_C5			0x55
#define MPU6000_REV_D6			0x56
#define MPU6000_REV_D7			0x57
#define MPU6000_REV_D8			0x58
#define MPU6000_REV_D9			0x59
#define MPU6000_REV_D10			0x5A
#define MPU6050_REV_D8			0x28	// TODO:Need verification

#define MPU6000_ACCEL_DEFAULT_RANGE_G				16

#define MPU6000_GYRO_DEFAULT_RANGE_G				8
#define MPU6000_GYRO_DEFAULT_RATE					1000


#define MPU6000_DEFAULT_ONCHIP_FILTER_FREQ			98

#pragma pack(push, 1)
/**
 * Report conversation within the MPU6000, including command byte and
 * interrupt status.
 */
struct MPUReport {
	uint8_t		cmd;
	uint8_t		status;
	uint8_t		accel_x[2];
	uint8_t		accel_y[2];
	uint8_t		accel_z[2];
	uint8_t		temp[2];
	uint8_t		gyro_x[2];
	uint8_t		gyro_y[2];
	uint8_t		gyro_z[2];
};
#pragma pack(pop)

#define MPU_MAX_READ_BUFFER_SIZE (sizeof(MPUReport) + 1)
#define MPU_MAX_WRITE_BUFFER_SIZE (2)
/*
  The MPU6000 can only handle high bus speeds on the sensor and
  interrupt status registers. All other registers have a maximum 1MHz
  Communication with all registers of the device is performed using either
  I2C at 400kHz or SPI at 1MHz. For applications requiring faster communications,
  the sensor and interrupt registers may be read using SPI at 20MHz
 */
#define MPU6000_LOW_BUS_SPEED				0
#define MPU6000_HIGH_BUS_SPEED				0x8000
#  define MPU6000_IS_HIGH_SPEED(r) 			((r) & MPU6000_HIGH_BUS_SPEED)
#  define MPU6000_REG(r) 					((r) &~MPU6000_HIGH_BUS_SPEED)
#  define MPU6000_SET_SPEED(r, s) 			((r)|(s))
#  define MPU6000_HIGH_SPEED_OP(r) 			MPU6000_SET_SPEED((r), MPU6000_HIGH_BUS_SPEED)
#  define MPU6000_LOW_SPEED_OP(r)			MPU6000_REG((r))


static struct __mpu6000 {
    struct spi_dev_s *bus;
} _mpu6000t;
static float _scale = 0.0f;
static struct __mpu6000 *mpu6000_handle_get()
{
    return &_mpu6000t;
}

int mpu6000_exchange(void *write, void *read, uint32_t size)
{
    int ret = 0;
    struct __mpu6000 *handle = mpu6000_handle_get();

	SPI_SETMODE(handle->bus, SPIDEV_MODE3);
	SPI_SETBITS(handle->bus, 8);
    SPI_SELECT(handle->bus, 0x21, true);
    SPI_EXCHANGEBLOCK(handle->bus, write, read, size);
    SPI_SELECT(handle->bus, 0x21, false);

    return ret;
}

uint8_t mpu6000_read_reg(uint8_t addr)
{
	uint8_t cmd[2] = {0};

	cmd[0] = addr | DIR_READ;
	cmd[1] = 0;

	mpu6000_exchange(cmd, cmd, sizeof(cmd));

	return cmd[1];
}

int mpu6000_write_reg(uint8_t addr, uint8_t data)
{
	uint8_t	cmd[2] = {0};

	cmd[0] = addr | DIR_WRITE;
	cmd[1] = data;

	return mpu6000_exchange(cmd, NULL, sizeof(cmd));
}

int mpu6000_drv_init(int spibus)
{
    uint8_t id = 0;
    uint8_t product = 0;
    bool unknown_product_id = true;
    struct __mpu6000 *handle = mpu6000_handle_get();
    if (!handle->bus) {
        handle->bus = spi_bus_get(spibus);
    }

    id = mpu6000_read_reg(MPUREG_WHOAMI);

    if (id != MPU_WHOAMI_6000 &&
        id != ICM_WHOAMI_20602 &&
        id != ICM_WHOAMI_20608 &&
        id != ICM_WHOAMI_20689) {
        return -1;
    }

    product = mpu6000_read_reg(MPUREG_PRODUCT_ID);

	switch (product) {
	case MPU6000ES_REV_C4:
	case MPU6000ES_REV_C5:
	case MPU6000_REV_C4:
	case MPU6000_REV_C5:
	case MPU6000ES_REV_D6:
	case MPU6000ES_REV_D7:
	case MPU6000ES_REV_D8:
	case MPU6000_REV_D6:
	case MPU6000_REV_D7:
	case MPU6000_REV_D8:
	case MPU6000_REV_D9:
	case MPU6000_REV_D10:
	case ICM20608_REV_FF:
	case ICM20689_REV_FE:
	case ICM20689_REV_03:
	case ICM20689_REV_04:
	case ICM20602_REV_01:
	case ICM20602_REV_02:
	case MPU6050_REV_D8:
		unknown_product_id = false;
	}

    if (unknown_product_id) {
        return -1;
    }

    return 0;
}
