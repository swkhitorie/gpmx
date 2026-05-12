#pragma once

#include <device/i2c_master.h>

#define NUMBER_I2C_BUSES  (5)

namespace device
{

class I2C
{
public:
	// no copy, assignment, move, move assignment
	I2C(const I2C &) = delete;
	I2C &operator=(const I2C &) = delete;
	I2C(I2C &&) = delete;
	I2C &operator=(I2C &&) = delete;

	virtual int	init();

	static int	set_bus_clock(unsigned bus, unsigned clock_hz);

protected:
	/** The number of times a read or write operation will be retried on error. */
	uint8_t		_retries{0};

	/**
	 * @ Constructor
	 *
	 * @param name		Driver name
	 * @param bus		I2C bus on which the device lives
	 * @param address	I2C bus address, or zero if set_address will be used
	 * @param frequency	I2C bus frequency for the device (currently not used)
	 */
	I2C(const char *name, const int bus, const uint16_t address, const uint32_t frequency);

	virtual ~I2C();

	/** Check for the presence of the device on the bus. */
	virtual int	probe() { return 0; }

	/** Perform an I2C transaction to the device. */
	int		transfer(const uint8_t *send, const unsigned send_len, uint8_t *recv, const unsigned recv_len);

protected:
	static unsigned	int	_bus_clocks[NUMBER_I2C_BUSES];
    const char	*_devname{nullptr};
    int             _bus;
    uint16_t        _address;
	const uint32_t		_frequency;
	struct i2c_master_s		*_dev{nullptr};

};

} // namespace device

