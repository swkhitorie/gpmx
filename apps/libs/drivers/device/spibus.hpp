#pragma once

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

protected:
	/**
	 * Constructor
	 *
	 * @param name		Driver name
	 * @param bus		SPI bus on which the device lives
	 * @param device	Device handle (used by SPI_SELECT)
	 * @param mode		SPI clock/data mode
	 * @param frequency	SPI clock frequency
	 */
	SPI(const char *name, int bus, uint32_t device, enum spi_mode_e mode, uint32_t frequency);

	virtual ~SPI();

	/** Locking modes supported by the driver. */
	enum LockMode {
		LOCK_PREEMPTION,	/**< the default; lock against all forms of preemption. */
		LOCK_THREADS,		/**< lock only against other threads, using SPI_LOCK */
		LOCK_NONE		/**< perform no locking, only safe if the bus is entirely private */
	};

	virtual int	init();

	/** Check for the presence of the device on the bus. */
	virtual int	probe() { return 0; }

	/** Perform a SPI transfer.*/
	int		transfer(uint8_t *send, uint8_t *recv, unsigned len);

	/** Perform a SPI 16 bit transfer.*/
	int		transferhword(uint16_t *send, uint16_t *recv, unsigned len);

	void		set_frequency(uint32_t frequency) { _frequency = frequency; }
	uint32_t	get_frequency() { return _frequency; }

	void		set_lockmode(enum LockMode mode) { _locking_mode = mode; }

protected:
    const char	*_devname{nullptr};
	uint32_t		_device;
    int             _bus;
	enum spi_mode_e		_mode;
	uint32_t		_frequency;
	struct spi_dev_s	*_dev {nullptr};

	LockMode		_locking_mode{LOCK_THREADS};	/**< selected locking mode */

	int	_transfer(uint8_t *send, uint8_t *recv, unsigned len);

	int	_transferhword(uint16_t *send, uint16_t *recv, unsigned len);

};

} // namespace device
