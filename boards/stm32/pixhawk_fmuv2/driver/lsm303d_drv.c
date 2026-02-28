#include "lsm303d_drv.h"
#include <board_config.h>
#include <device/spi.h>

#ifndef TEST_PRINTF
#define TEST_PRINTF    BOARD_PRINTF
#endif


#ifndef CONSTANTS_ONE_G
#define CONSTANTS_ONE_G  9.78
#endif

/* SPI protocol address bits */
#define DIR_READ			(1<<7)
#define DIR_WRITE			(0<<7)
#define ADDR_INCREMENT			(1<<6)

/* register addresses: A: accel, M: mag, T: temp */
#define ADDR_WHO_AM_I			0x0F
#define WHO_I_AM			0x49

#define ADDR_OUT_TEMP_L			0x05

#define ADDR_STATUS_A			0x27

#define ADDR_CTRL_REG0			0x1F
#define ADDR_CTRL_REG1			0x20
#define ADDR_CTRL_REG2			0x21
#define ADDR_CTRL_REG3			0x22
#define ADDR_CTRL_REG4			0x23
#define ADDR_CTRL_REG5			0x24
#define ADDR_CTRL_REG6			0x25
#define ADDR_CTRL_REG7			0x26

#define REG1_RATE_BITS_A		((1<<7) | (1<<6) | (1<<5) | (1<<4))
#define REG1_POWERDOWN_A		((0<<7) | (0<<6) | (0<<5) | (0<<4))
#define REG1_RATE_3_125HZ_A		((0<<7) | (0<<6) | (0<<5) | (1<<4))
#define REG1_RATE_6_25HZ_A		((0<<7) | (0<<6) | (1<<5) | (0<<4))
#define REG1_RATE_12_5HZ_A		((0<<7) | (0<<6) | (1<<5) | (1<<4))
#define REG1_RATE_25HZ_A		((0<<7) | (1<<6) | (0<<5) | (0<<4))
#define REG1_RATE_50HZ_A		((0<<7) | (1<<6) | (0<<5) | (1<<4))
#define REG1_RATE_100HZ_A		((0<<7) | (1<<6) | (1<<5) | (0<<4))
#define REG1_RATE_200HZ_A		((0<<7) | (1<<6) | (1<<5) | (1<<4))
#define REG1_RATE_400HZ_A		((1<<7) | (0<<6) | (0<<5) | (0<<4))
#define REG1_RATE_800HZ_A		((1<<7) | (0<<6) | (0<<5) | (1<<4))
#define REG1_RATE_1600HZ_A		((1<<7) | (0<<6) | (1<<5) | (0<<4))

#define REG1_BDU_UPDATE			(1<<3)
#define REG1_Z_ENABLE_A			(1<<2)
#define REG1_Y_ENABLE_A			(1<<1)
#define REG1_X_ENABLE_A			(1<<0)

#define REG2_ANTIALIAS_FILTER_BW_BITS_A	((1<<7) | (1<<6))
#define REG2_AA_FILTER_BW_773HZ_A		((0<<7) | (0<<6))
#define REG2_AA_FILTER_BW_194HZ_A		((0<<7) | (1<<6))
#define REG2_AA_FILTER_BW_362HZ_A		((1<<7) | (0<<6))
#define REG2_AA_FILTER_BW_50HZ_A		((1<<7) | (1<<6))

#define REG2_FULL_SCALE_BITS_A	((1<<5) | (1<<4) | (1<<3))
#define REG2_FULL_SCALE_2G_A	((0<<5) | (0<<4) | (0<<3))
#define REG2_FULL_SCALE_4G_A	((0<<5) | (0<<4) | (1<<3))
#define REG2_FULL_SCALE_6G_A	((0<<5) | (1<<4) | (0<<3))
#define REG2_FULL_SCALE_8G_A	((0<<5) | (1<<4) | (1<<3))
#define REG2_FULL_SCALE_16G_A	((1<<5) | (0<<4) | (0<<3))

#define REG5_ENABLE_T			(1<<7)

#define REG5_RES_HIGH_M			((1<<6) | (1<<5))
#define REG5_RES_LOW_M			((0<<6) | (0<<5))

#define REG5_RATE_BITS_M		((1<<4) | (1<<3) | (1<<2))
#define REG5_RATE_3_125HZ_M		((0<<4) | (0<<3) | (0<<2))
#define REG5_RATE_6_25HZ_M		((0<<4) | (0<<3) | (1<<2))
#define REG5_RATE_12_5HZ_M		((0<<4) | (1<<3) | (0<<2))
#define REG5_RATE_25HZ_M		((0<<4) | (1<<3) | (1<<2))
#define REG5_RATE_50HZ_M		((1<<4) | (0<<3) | (0<<2))
#define REG5_RATE_100HZ_M		((1<<4) | (0<<3) | (1<<2))
#define REG5_RATE_DO_NOT_USE_M	((1<<4) | (1<<3) | (0<<2))

#define REG6_FULL_SCALE_BITS_M	((1<<6) | (1<<5))
#define REG6_FULL_SCALE_2GA_M	((0<<6) | (0<<5))
#define REG6_FULL_SCALE_4GA_M	((0<<6) | (1<<5))
#define REG6_FULL_SCALE_8GA_M	((1<<6) | (0<<5))
#define REG6_FULL_SCALE_12GA_M	((1<<6) | (1<<5))

#define REG7_CONT_MODE_M		((0<<1) | (0<<0))

#define REG_STATUS_A_NEW_ZYXADA		0x08

/* default values for this device */
#define LSM303D_ACCEL_DEFAULT_RANGE_G			16
#define LSM303D_ACCEL_DEFAULT_RATE			800
#define LSM303D_ACCEL_DEFAULT_ONCHIP_FILTER_FREQ	50

#define LSM303D_MAG_DEFAULT_RANGE_GA			2
#define LSM303D_MAG_DEFAULT_RATE			100

static struct __lsmddrv {
    struct spi_dev_s *bus;
} _lsmimu;

static float accel_range_scale = 0.0f;
static float _call_accel_interval = 0.0f;
static float scale_ga_digit = 0.0f;
static float _call_mag_interval = 0.0f;
static int16_t _last_accel[3];
static int _constant_accel_count = 0;

static struct __lsmddrv *lsm_handle_get()
{
    return &_lsmimu;
}

int lsm303d_exchange(void *write, void *read, uint32_t size)
{
    int ret = 0;
    struct __lsmddrv *handle = lsm_handle_get();

	SPI_SETMODE(handle->bus, SPIDEV_MODE0);
	SPI_SETBITS(handle->bus, 8);
    SPI_SELECT(handle->bus, 0x11, true);
    SPI_EXCHANGEBLOCK(handle->bus, write, read, size);
    SPI_SELECT(handle->bus, 0x11, false);

    return ret;
}

uint8_t lsm303d_read_reg(uint8_t addr)
{
	uint8_t cmd[2] = {0};
	cmd[0] = addr | DIR_READ;

	lsm303d_exchange(cmd, cmd, sizeof(cmd));

	return cmd[1];
}

int lsm303d_write_reg(uint8_t addr, uint8_t data)
{
	uint8_t	cmd[2] = {0};

	cmd[0] = addr | DIR_WRITE;
	cmd[1] = data;

	return lsm303d_exchange(cmd, NULL, sizeof(cmd));
}

void lsm303d_modify_reg(unsigned reg, uint8_t clearbits, uint8_t setbits)
{
	uint8_t	val = lsm303d_read_reg(reg);
	val &= ~clearbits;
	val |= setbits;
	lsm303d_write_reg(reg, val);
}

int lsm303d_accel_set_range(unsigned max_g)
{
	uint8_t setbits = 0;
	uint8_t clearbits = REG2_FULL_SCALE_BITS_A;
	float new_scale_g_digit = 0.0f;

	if (max_g == 0) {
		max_g = 16;
	}

	if (max_g <= 2) {
		// accel_range_m_s2 = 2.0f * CONSTANTS_ONE_G;
		setbits |= REG2_FULL_SCALE_2G_A;
		new_scale_g_digit = 0.061e-3f;

	} else if (max_g <= 4) {
		// accel_range_m_s2 = 4.0f * CONSTANTS_ONE_G;
		setbits |= REG2_FULL_SCALE_4G_A;
		new_scale_g_digit = 0.122e-3f;

	} else if (max_g <= 6) {
		// accel_range_m_s2 = 6.0f * CONSTANTS_ONE_G;
		setbits |= REG2_FULL_SCALE_6G_A;
		new_scale_g_digit = 0.183e-3f;

	} else if (max_g <= 8) {
		// accel_range_m_s2 = 8.0f * CONSTANTS_ONE_G;
		setbits |= REG2_FULL_SCALE_8G_A;
		new_scale_g_digit = 0.244e-3f;

	} else if (max_g <= 16) {
		// accel_range_m_s2 = 16.0f * CONSTANTS_ONE_G;
		setbits |= REG2_FULL_SCALE_16G_A;
		new_scale_g_digit = 0.732e-3f;

	} else {
		return -1;
	}

	accel_range_scale = new_scale_g_digit * CONSTANTS_ONE_G;

	lsm303d_modify_reg(ADDR_CTRL_REG2, clearbits, setbits);

	return 0;
}

int lsm303d_accel_set_samplerate(unsigned frequency)
{
	uint8_t setbits = 0;
	uint8_t clearbits = REG1_RATE_BITS_A;

	if (frequency == 0) {
		frequency = 1600;
	}

	int accel_samplerate = 100;

	if (frequency <= 100) {
		setbits |= REG1_RATE_100HZ_A;
		accel_samplerate = 100;

	} else if (frequency <= 200) {
		setbits |= REG1_RATE_200HZ_A;
		accel_samplerate = 200;

	} else if (frequency <= 400) {
		setbits |= REG1_RATE_400HZ_A;
		accel_samplerate = 400;

	} else if (frequency <= 800) {
		setbits |= REG1_RATE_800HZ_A;
		accel_samplerate = 800;

	} else if (frequency <= 1600) {
		setbits |= REG1_RATE_1600HZ_A;
		accel_samplerate = 1600;

	} else {
		return -1;
	}

	_call_accel_interval = 1000000 / accel_samplerate;

	lsm303d_modify_reg(ADDR_CTRL_REG1, clearbits, setbits);

	return 0;
}

int lsm303d_accel_set_onchip_lowpass_filter_bandwidth(unsigned bandwidth)
{
	uint8_t setbits = 0;
	uint8_t clearbits = REG2_ANTIALIAS_FILTER_BW_BITS_A;

	if (bandwidth == 0) {
		bandwidth = 773;
	}

	if (bandwidth <= 50) {
		// accel_onchip_filter_bandwith = 50;
		setbits |= REG2_AA_FILTER_BW_50HZ_A;

	} else if (bandwidth <= 194) {
		// accel_onchip_filter_bandwith = 194;
		setbits |= REG2_AA_FILTER_BW_194HZ_A;

	} else if (bandwidth <= 362) {
		// accel_onchip_filter_bandwith = 362;
		setbits |= REG2_AA_FILTER_BW_362HZ_A;

	} else if (bandwidth <= 773) {
		// accel_onchip_filter_bandwith = 773;
		setbits |= REG2_AA_FILTER_BW_773HZ_A;

	} else {
		return -1;
	}

	lsm303d_modify_reg(ADDR_CTRL_REG2, clearbits, setbits);

	return 0;
}

int lsm303d_mag_set_range(unsigned max_ga)
{
	uint8_t setbits = 0;
	uint8_t clearbits = REG6_FULL_SCALE_BITS_M;
	float new_scale_ga_digit = 0.0f;

	if (max_ga == 0) {
		max_ga = 12;
	}

	if (max_ga <= 2) {
		// mag_range_ga = 2;
		setbits |= REG6_FULL_SCALE_2GA_M;
		new_scale_ga_digit = 0.080e-3f;

	} else if (max_ga <= 4) {
		// mag_range_ga = 4;
		setbits |= REG6_FULL_SCALE_4GA_M;
		new_scale_ga_digit = 0.160e-3f;

	} else if (max_ga <= 8) {
		// mag_range_ga = 8;
		setbits |= REG6_FULL_SCALE_8GA_M;
		new_scale_ga_digit = 0.320e-3f;

	} else if (max_ga <= 12) {
		// mag_range_ga = 12;
		setbits |= REG6_FULL_SCALE_12GA_M;
		new_scale_ga_digit = 0.479e-3f;

	} else {
		return -1;
	}

    scale_ga_digit = new_scale_ga_digit;

	lsm303d_modify_reg(ADDR_CTRL_REG6, clearbits, setbits);

	return 0;
}

int lsm303d_mag_set_samplerate(unsigned frequency)
{
	uint8_t setbits = 0;
	uint8_t clearbits = REG5_RATE_BITS_M;

	if (frequency == 0) {
		frequency = 100;
	}

	int mag_samplerate = 100;

	if (frequency <= 25) {
		setbits |= REG5_RATE_25HZ_M;
		mag_samplerate = 25;

	} else if (frequency <= 50) {
		setbits |= REG5_RATE_50HZ_M;
		mag_samplerate = 50;

	} else if (frequency <= 100) {
		setbits |= REG5_RATE_100HZ_M;
		mag_samplerate = 100;

	} else {
		return -1;
	}

	_call_mag_interval = 1000000 / mag_samplerate;

	lsm303d_modify_reg(ADDR_CTRL_REG5, clearbits, setbits);

	return 0;
}

void lsm303d_reset()
{
	// enable accel
	lsm303d_write_reg(ADDR_CTRL_REG1, REG1_X_ENABLE_A | REG1_Y_ENABLE_A | REG1_Z_ENABLE_A | REG1_BDU_UPDATE |
			  REG1_RATE_800HZ_A);

	// enable mag
	lsm303d_write_reg(ADDR_CTRL_REG7, REG7_CONT_MODE_M);
	lsm303d_write_reg(ADDR_CTRL_REG5, REG5_RES_HIGH_M | REG5_ENABLE_T);
	lsm303d_write_reg(ADDR_CTRL_REG3, 0x04); // DRDY on ACCEL on INT1
	lsm303d_write_reg(ADDR_CTRL_REG4, 0x04); // DRDY on MAG on INT2

	lsm303d_accel_set_range(LSM303D_ACCEL_DEFAULT_RANGE_G);
	lsm303d_accel_set_samplerate(LSM303D_ACCEL_DEFAULT_RATE);

	// we setup the anti-alias on-chip filter as 50Hz. We believe
	// this operates in the analog domain, and is critical for
	// anti-aliasing. The 2 pole software filter is designed to
	// operate in conjunction with this on-chip filter
	lsm303d_accel_set_onchip_lowpass_filter_bandwidth(LSM303D_ACCEL_DEFAULT_ONCHIP_FILTER_FREQ);

	lsm303d_mag_set_range(LSM303D_MAG_DEFAULT_RANGE_GA);
	lsm303d_mag_set_samplerate(LSM303D_MAG_DEFAULT_RATE);
}

int lsm303d_drv_init(int spibus)
{
    struct __lsmddrv *handle = lsm_handle_get();
    if (!handle->bus) {
        handle->bus = spi_bus_get(spibus);
    }

	lsm303d_read_reg(ADDR_WHO_AM_I);

	// verify that the device is attached and functioning
	if (lsm303d_read_reg(ADDR_WHO_AM_I) != WHO_I_AM) {
		return -1;
	}

    lsm303d_reset();

    return 0;
}

int lsm303d_test_accel()
{
	// status register and data as read back from the device
#pragma pack(push, 1)
	struct {
		uint8_t		cmd;
		uint8_t		status;
		int16_t		x;
		int16_t		y;
		int16_t		z;
	} raw_accel_report = {0};
#pragma pack(pop)

	raw_accel_report.cmd = ADDR_STATUS_A | DIR_READ | ADDR_INCREMENT;
	lsm303d_exchange((uint8_t *)&raw_accel_report, (uint8_t *)&raw_accel_report, sizeof(raw_accel_report));

	if (!(raw_accel_report.status & REG_STATUS_A_NEW_ZYXADA)) {
        TEST_PRINTF("[lsm303d] accel error :%d \r\n", raw_accel_report.status);
		return -1;
	}

	if (((_last_accel[0] - raw_accel_report.x) == 0) &&
        ((_last_accel[1] - raw_accel_report.y) == 0) &&
        ((_last_accel[2] - raw_accel_report.z) == 0)) {
		_constant_accel_count++;
	} else {
		_constant_accel_count = 0;
	}

	if (_constant_accel_count > 100) {
		return -2;
	}

    TEST_PRINTF("[lsm303d] accel x:%d, y:%d, z:%d\r\n", raw_accel_report.x, raw_accel_report.y, raw_accel_report.z);

	_last_accel[0] = raw_accel_report.x;
	_last_accel[1] = raw_accel_report.y;
	_last_accel[2] = raw_accel_report.z;

    return 0;
}

int lsm303d_test_mag()
{
	// status register and data as read back from the device
#pragma pack(push, 1)
	struct {
		uint8_t		cmd;
		int16_t		temperature;
		uint8_t		status;
		int16_t		x;
		int16_t		y;
		int16_t		z;
	} raw_mag_report = {0};
#pragma pack(pop)

	raw_mag_report.cmd = ADDR_OUT_TEMP_L | DIR_READ | ADDR_INCREMENT;
	lsm303d_exchange((uint8_t *)&raw_mag_report, (uint8_t *)&raw_mag_report, sizeof(raw_mag_report));

    TEST_PRINTF("[lsm303d] mag x:%d, y:%d, z:%d\r\n", raw_mag_report.x, raw_mag_report.y, raw_mag_report.z);

    return 0;
}
