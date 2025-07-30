#ifndef _GPDEV_SPI_H
#define _GPDEV_SPI_H

#include <device/dnode.h>
#include <device/spi.h>

namespace device
{

class SPI
{
public:
	// no copy, assignment, move, move assignment
	SPI(const SPI &) = delete;
	SPI &operator=(const SPI &) = delete;
	SPI(SPI &&) = delete;
	SPI &operator=(SPI &&) = delete;

	virtual int	init();

protected:
	/**
	 * Constructor
	 *
	 * @param name		Driver name
	 * @param bus		SPI bus name on which the device lives
	 * @param device	Device handle (used by SPI_SELECT)
	 * @param mode		SPI clock/data mode
	 * @param frequency	SPI clock frequency
	 */
	SPI(const char *name, const char *bus, uint32_t device, enum spi_mode_e mode, uint32_t frequency);
	virtual ~SPI();

	/**
	 * Locking modes supported by the driver.
	 */
	enum LockMode {
		LOCK_PREEMPTION,	/**< the default; lock against all forms of preemption. */
		LOCK_THREADS,		/**< lock only against other threads, using SPI_LOCK */
		LOCK_NONE		/**< perform no locking, only safe if the bus is entirely private */
	};

	/**
	 * Check for the presence of the device on the bus.
	 */
	virtual int	probe() { return PX4_OK; }

	int		transfer(uint8_t *send, uint8_t *recv, unsigned len);

	/**
	 * Set the SPI bus frequency
	 * This is used to change frequency on the fly. Some sensors
	 * (such as the MPU6000) need a lower frequency for setup
	 * registers and can handle higher frequency for sensor
	 * value registers
	 *
	 * @param frequency	Frequency to set (Hz)
	 */
	void		set_frequency(uint32_t frequency) { _frequency = frequency; }
	uint32_t	get_frequency() { return _frequency; }

	/**
	 * Set the SPI bus locking mode
	 *
	 * This set the SPI locking mode. For devices competing with NuttX SPI
	 * drivers on a bus the right lock mode is LOCK_THREADS.
	 *
	 * @param mode	Locking mode
	 */
	void		set_lockmode(enum LockMode mode) { _locking_mode = mode; }

private:
	uint32_t        _device;

	enum spi_mode_e		_mode;
	uint32_t		_frequency;

	const char  _devname[16] {};		/**< device node name */

    const char  _busname[16] {};		/**< i2c bus name */

	struct spi_dev_s	*_dev {nullptr};

	LockMode		_locking_mode{LOCK_THREADS};	/**< selected locking mode */

protected:
	int	_transfer(uint8_t *send, uint8_t *recv, unsigned len);
};

} // namespace device

#endif /* _GPDEV_SPI_H */
