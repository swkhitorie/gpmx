#pragma once

#include <dev/i2c_master.h>

#define HMC5883L_ADDRESS		0x1E

#define ADDR_ID_A			0x0a
#define ADDR_ID_B			0x0b
#define ADDR_ID_C			0x0c

#define ID_A_WHO_AM_I			'H'
#define ID_B_WHO_AM_I			'4'
#define ID_C_WHO_AM_I			'3'

/* HMC5883 internal constants and data structures. */

/* Max measurement rate is 160Hz, however with 160 it will be set to 166 Hz, therefore workaround using 150 */
#define HMC5883_CONVERSION_INTERVAL   (1e6f / 150)  /* microseconds */

#define ADDR_CONF_A             0x00
#define ADDR_CONF_B             0x01
#define ADDR_MODE               0x02
#define ADDR_DATA_OUT_X_MSB     0x03
#define ADDR_DATA_OUT_X_LSB     0x04
#define ADDR_DATA_OUT_Z_MSB     0x05
#define ADDR_DATA_OUT_Z_LSB     0x06
#define ADDR_DATA_OUT_Y_MSB     0x07
#define ADDR_DATA_OUT_Y_LSB     0x08
#define ADDR_STATUS             0x09

/* temperature on hmc5983 only */
#define ADDR_TEMP_OUT_MSB       0x31
#define ADDR_TEMP_OUT_LSB       0x32

/* modes not changeable outside of driver */
#define HMC5883L_MODE_NORMAL		(0 << 0)  /* default */
#define HMC5883L_MODE_POSITIVE_BIAS	(1 << 0)  /* positive bias */
#define HMC5883L_MODE_NEGATIVE_BIAS	(1 << 1)  /* negative bias */

#define HMC5883L_AVERAGING_1		(0 << 5) /* conf a register */
#define HMC5883L_AVERAGING_2		(1 << 5)
#define HMC5883L_AVERAGING_4		(2 << 5)
#define HMC5883L_AVERAGING_8		(3 << 5)

#define MODE_REG_CONTINOUS_MODE		(0 << 0)
#define MODE_REG_SINGLE_MODE		(1 << 0) /* default */

#define STATUS_REG_DATA_OUT_LOCK	(1 << 1) /* page 16: set if data is only partially read, read device to reset */
#define STATUS_REG_DATA_READY		(1 << 0) /* page 16: set if all axes have valid measurements */

#define HMC5983_TEMP_SENSOR_ENABLE	(1 << 7)

class HMC5883
{
public:
	HMC5883(device::Device *interface, const I2CSPIDriverConfig &config);
	~HMC5883();

	static I2CSPIDriverBase *instantiate(const I2CSPIDriverConfig &config, int runtime_instance);
	static void print_usage();

	void			RunImpl();

	int		init();

protected:
	void print_status() override;

private:
	PX4Magnetometer		_px4_mag;
	device::Device		*_interface;
	unsigned		_measure_interval{0};

	float 			_range_ga;
	bool			_collect_phase;

	perf_counter_t		_sample_perf;
	perf_counter_t		_comms_errors;
	perf_counter_t		_range_errors;
	perf_counter_t		_conf_errors;

	uint8_t			_range_bits;
	uint8_t			_conf_reg;
	uint8_t			_temperature_counter;
	uint8_t			_temperature_error_count;

	/**
	 * Initialise the automatic measurement state machine and start it.
	 *
	 * @note This function is called at open and error time.  It might make sense
	 *       to make it more aggressive about resetting the bus in case of errors.
	 */
	void			start();

	int			reset();

	int			set_temperature_compensation(unsigned enable);

	int 			set_range(unsigned range);

	void 			check_range();

	void 			check_conf();

	int			write_reg(uint8_t reg, uint8_t val);

	int			read_reg(uint8_t reg, uint8_t &val);

	int			measure();

	int			collect();
};
