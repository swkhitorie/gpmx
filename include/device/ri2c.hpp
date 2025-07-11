#ifndef _DEVICE_I2C_H
#define _DEVICE_I2C_H

#include <device/dnode.h>
#include <device/i2c_master.h>

namespace device
{

class RI2C
{

public:

	// no copy, assignment, move, move assignment
	RI2C(const RI2C &) = delete;
	RI2C &operator=(const RI2C &) = delete;
	RI2C(RI2C &&) = delete;
	RI2C &operator=(RI2C &&) = delete;

	virtual int	init();

protected:
	/**
	 * The number of times a read or write operation will be retried on
	 * error.
	 */
	uint8_t		_retries{0};

	/**
	 * @ Constructor
	 *
	 * @param name		Driver name
	 * @param bus		I2C bus name on which the device lives
	 * @param address	I2C bus address, or zero if set_address will be used
	 * @param frequency	I2C bus frequency for the device (currently not used)
	 */
	RI2C(const char *name, const char *bus, const uint16_t address, const uint32_t frequency);
	virtual ~RI2C();

	/**
	 * Check for the presence of the device on the bus.
	 */
	virtual int	probe() { return DEV_OK; }

	int		transfer(const uint8_t regsz, 
        const uint8_t *send, const unsigned send_len,
        uint8_t *recv, const unsigned recv_len);

private:
	const uint32_t		_frequency;
    const uint16_t   _address;

	const char  _devname[16] {};		/**< device node name */

    const char  _busname[16] {};		/**< i2c bus name */

	struct i2c_master_s		*_dev{nullptr};
};

} // namespace device

#endif
