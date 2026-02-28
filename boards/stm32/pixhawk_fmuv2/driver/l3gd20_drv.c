#include "l3gd20_drv.h"
#include <board_config.h>
#include <device/spi.h>

#ifndef TEST_PRINTF
#define TEST_PRINTF    BOARD_PRINTF
#endif


/* SPI protocol address bits */
#define DIR_READ				(1<<7)
#define DIR_WRITE				(0<<7)
#define ADDR_INCREMENT				(1<<6)

/* register addresses */
#define ADDR_WHO_AM_I			0x0F
#define WHO_I_AM_H 				0xD7
#define WHO_I_AM				0xD4
#define WHO_I_AM_L3G4200D		0xD3	/* for L3G4200D */

#define ADDR_CTRL_REG1			0x20
#define REG1_RATE_LP_MASK			0xF0 /* Mask to guard partial register update */

/* keep lowpass low to avoid noise issues */
#define RATE_95HZ_LP_25HZ		((0<<7) | (0<<6) | (0<<5) | (1<<4))
#define RATE_190HZ_LP_25HZ		((0<<7) | (1<<6) | (0<<5) | (1<<4))
#define RATE_190HZ_LP_50HZ		((0<<7) | (1<<6) | (1<<5) | (0<<4))
#define RATE_190HZ_LP_70HZ		((0<<7) | (1<<6) | (1<<5) | (1<<4))
#define RATE_380HZ_LP_20HZ		((1<<7) | (0<<6) | (1<<5) | (0<<4))
#define RATE_380HZ_LP_25HZ		((1<<7) | (0<<6) | (0<<5) | (1<<4))
#define RATE_380HZ_LP_50HZ		((1<<7) | (0<<6) | (1<<5) | (0<<4))
#define RATE_380HZ_LP_100HZ		((1<<7) | (0<<6) | (1<<5) | (1<<4))
#define RATE_760HZ_LP_30HZ		((1<<7) | (1<<6) | (0<<5) | (0<<4))
#define RATE_760HZ_LP_35HZ		((1<<7) | (1<<6) | (0<<5) | (1<<4))
#define RATE_760HZ_LP_50HZ		((1<<7) | (1<<6) | (1<<5) | (0<<4))
#define RATE_760HZ_LP_100HZ		((1<<7) | (1<<6) | (1<<5) | (1<<4))

#define ADDR_CTRL_REG2			0x21
#define ADDR_CTRL_REG3			0x22
#define ADDR_CTRL_REG4			0x23
#define REG4_RANGE_MASK				0x30 /* Mask to guard partial register update */
#define RANGE_250DPS				(0<<4)
#define RANGE_500DPS				(1<<4)
#define RANGE_2000DPS				(3<<4)

#define ADDR_CTRL_REG5			0x24
#define ADDR_REFERENCE			0x25
#define ADDR_OUT_TEMP			0x26
#define ADDR_STATUS_REG			0x27
#define ADDR_OUT_X_L			0x28
#define ADDR_OUT_X_H			0x29
#define ADDR_OUT_Y_L			0x2A
#define ADDR_OUT_Y_H			0x2B
#define ADDR_OUT_Z_L			0x2C
#define ADDR_OUT_Z_H			0x2D
#define ADDR_FIFO_CTRL_REG		0x2E
#define ADDR_FIFO_SRC_REG		0x2F
#define ADDR_INT1_CFG			0x30
#define ADDR_INT1_SRC			0x31
#define ADDR_INT1_TSH_XH		0x32
#define ADDR_INT1_TSH_XL		0x33
#define ADDR_INT1_TSH_YH		0x34
#define ADDR_INT1_TSH_YL		0x35
#define ADDR_INT1_TSH_ZH		0x36
#define ADDR_INT1_TSH_ZL		0x37
#define ADDR_INT1_DURATION		0x38
#define ADDR_LOW_ODR			0x39


/* Internal configuration values */
#define REG1_POWER_NORMAL			(1<<3)
#define REG1_Z_ENABLE				(1<<2)
#define REG1_Y_ENABLE				(1<<1)
#define REG1_X_ENABLE				(1<<0)

#define REG4_BDU				(1<<7)
#define REG4_BLE				(1<<6)
//#define REG4_SPI_3WIRE			(1<<0)

#define REG5_FIFO_ENABLE			(1<<6)
#define REG5_REBOOT_MEMORY			(1<<7)

#define STATUS_ZYXOR				(1<<7)
#define STATUS_ZOR				(1<<6)
#define STATUS_YOR				(1<<5)
#define STATUS_XOR				(1<<4)
#define STATUS_ZYXDA				(1<<3)
#define STATUS_ZDA				(1<<2)
#define STATUS_YDA				(1<<1)
#define STATUS_XDA				(1<<0)

#define FIFO_CTRL_BYPASS_MODE			(0<<5)
#define FIFO_CTRL_FIFO_MODE			(1<<5)
#define FIFO_CTRL_STREAM_MODE			(1<<6)
#define FIFO_CTRL_STREAM_TO_FIFO_MODE		(3<<5)
#define FIFO_CTRL_BYPASS_TO_STREAM_MODE		(1<<7)

#define L3GD20_DEFAULT_RATE			760
#define L3GD20_DEFAULT_RANGE_DPS		2000
#define L3GD20_TEMP_OFFSET_CELSIUS		40

static struct __l3gdrv {
    struct spi_dev_s *bus;
} _l3gimu;
static bool _is_l3g4200d = false;
static float _scale = 0.0f;

static struct __l3gdrv *l3gd_handle_get()
{
    return &_l3gimu;
}

int l3gd20_exchange(void *write, void *read, uint32_t size)
{
    int ret = 0;
    struct __l3gdrv *handle = l3gd_handle_get();

	SPI_SETMODE(handle->bus, SPIDEV_MODE0);
	SPI_SETBITS(handle->bus, 8);
    SPI_SELECT(handle->bus, 0x22, true);
    SPI_EXCHANGEBLOCK(handle->bus, write, read, size);
    SPI_SELECT(handle->bus, 0x22, false);

    return ret;
}

uint8_t l3gd20_read_reg(uint8_t addr)
{
	uint8_t cmd[2] = {0};

	cmd[0] = addr | DIR_READ;
	cmd[1] = 0;

	l3gd20_exchange(cmd, cmd, sizeof(cmd));

	return cmd[1];
}

int l3gd20_write_reg(uint8_t addr, uint8_t data)
{
	uint8_t	cmd[2] = {0};

	cmd[0] = addr | DIR_WRITE;
	cmd[1] = data;

	return l3gd20_exchange(cmd, NULL, sizeof(cmd));
}

int l3gd20_set_samplerate(unsigned frequency)
{
	uint8_t bits = REG1_POWER_NORMAL | REG1_Z_ENABLE | REG1_Y_ENABLE | REG1_X_ENABLE;

	if (frequency == 0) {
		frequency = _is_l3g4200d ? 800 : 760;
	}

	/*
	 * Use limits good for H or non-H models. Rates are slightly different
	 * for L3G4200D part but register settings are the same.
	 */
	if (frequency <= 100) {
		bits |= RATE_95HZ_LP_25HZ;

	} else if (frequency <= 200) {
		bits |= RATE_190HZ_LP_50HZ;

	} else if (frequency <= 400) {
		bits |= RATE_380HZ_LP_50HZ;

	} else if (frequency <= 800) {
		bits |= RATE_760HZ_LP_50HZ;

	} else {
		return -1;
	}

	l3gd20_write_reg(ADDR_CTRL_REG1, bits);

	return 0;
}

int l3gd20_set_range(unsigned max_dps)
{
	uint8_t bits = REG4_BDU;
	float new_range_scale_dps_digit;

	if (max_dps == 0) {
		max_dps = 2000;
	}

	if (max_dps <= 250) {
		//new_range = 250;
		bits |= RANGE_250DPS;
		new_range_scale_dps_digit = 8.75e-3f;

	} else if (max_dps <= 500) {
		//new_range = 500;
		bits |= RANGE_500DPS;
		new_range_scale_dps_digit = 17.5e-3f;

	} else if (max_dps <= 2000) {
		//new_range = 2000;
		bits |= RANGE_2000DPS;
		new_range_scale_dps_digit = 70e-3f;

	} else {
		return -1;
	}

    _scale = new_range_scale_dps_digit / 180.0f * 3.141592654;

	l3gd20_write_reg(ADDR_CTRL_REG4, bits);

	return 0;
}

void l3gd20_reset()
{
	/* set default configuration */
	l3gd20_write_reg(ADDR_CTRL_REG1, REG1_POWER_NORMAL | REG1_Z_ENABLE | REG1_Y_ENABLE | REG1_X_ENABLE);
	l3gd20_write_reg(ADDR_CTRL_REG2, 0);		/* disable high-pass filters */
	l3gd20_write_reg(ADDR_CTRL_REG3, 0x08);        /* DRDY enable */
	l3gd20_write_reg(ADDR_CTRL_REG4, REG4_BDU);
	l3gd20_write_reg(ADDR_CTRL_REG5, 0);
	l3gd20_write_reg(ADDR_CTRL_REG5, REG5_FIFO_ENABLE);		/* disable wake-on-interrupt */

	/* disable FIFO. This makes things simpler and ensures we
	 * aren't getting stale data. It means we must run the hrt
	 * callback fast enough to not miss data. */
	l3gd20_write_reg(ADDR_FIFO_CTRL_REG, FIFO_CTRL_BYPASS_MODE);

	l3gd20_set_samplerate(0); // 760Hz or 800Hz
	l3gd20_set_range(L3GD20_DEFAULT_RANGE_DPS);
}

int l3gd20_drv_init(int spibus)
{
    uint8_t id = 0;
    struct __l3gdrv *handle = l3gd_handle_get();
    if (!handle->bus) {
        handle->bus = spi_bus_get(spibus);
    }

    id = l3gd20_read_reg(0x0F);

    if (id == WHO_I_AM) {
        _is_l3g4200d = false;
    } else if (id == WHO_I_AM) {
        _is_l3g4200d = false;
    } else if (id == WHO_I_AM_L3G4200D) {
        _is_l3g4200d = true;
    } else {
        return -1;
    }

    l3gd20_reset();

    return 0;
}

void l3gd20_test()
{
	/* status register and data as read back from the device */
#pragma pack(push, 1)
	struct {
		uint8_t		cmd;
		int8_t		temp;
		uint8_t		status;
		int16_t		x;
		int16_t		y;
		int16_t		z;
	} raw_report = {0};
#pragma pack(pop)

	raw_report.cmd = ADDR_OUT_TEMP | DIR_READ | ADDR_INCREMENT;
    l3gd20_exchange((uint8_t *)&raw_report, (uint8_t *)&raw_report, sizeof(raw_report));

	if (!(raw_report.status & STATUS_ZYXDA)) {
        TEST_PRINTF("[l3gd20] error :%d \r\n", raw_report.status);
		return;
	}

	TEST_PRINTF("[l3gd20] x:%d, y:%d, z:%d\r\n", raw_report.x, raw_report.y, raw_report.z);
}
