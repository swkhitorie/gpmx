#include "spibus.hpp"
#include <cstring>

namespace device
{

SPI::SPI(const char *name, int bus, uint32_t device, enum spi_mode_e mode, uint32_t frequency) :
	_device(device),
    _bus(bus),
	_mode(mode),
	_frequency(frequency)
{
    _locking_mode = LOCK_NONE;
	strncpy(_devname, name, 16);
}

SPI::~SPI()
{
    
}

int SPI::init()
{
	/* attach to the spi bus */
	if (_dev == nullptr) {
		_dev = spi_bus_get(_bus);
	}

	if (_dev == nullptr) {
		return -1;
	}

	/* deselect device to ensure high to low transition of pin select */
	SPI_SELECT(_dev, _device, false);

	/* call the probe function to check whether the device is present */
	int ret = probe();

	if (ret != 0) {
		return ret;
	}

	return 0;
}

int SPI::transfer(uint8_t *send, uint8_t *recv, unsigned len)
{
	int result;

	if ((send == nullptr) && (recv == nullptr)) {
		return -1;
	}

	LockMode mode = up_interrupt_context() ? LOCK_NONE : _locking_mode;

	switch (mode) {
	default:
	case LOCK_PREEMPTION: {
			int state = enter_critical_section();
			result = _transfer(send, recv, len);
			leave_critical_section(state);
		}
		break;

	case LOCK_THREADS:
		SPI_LOCK(_dev, true);
		result = _transfer(send, recv, len);
		SPI_LOCK(_dev, false);
		break;

	case LOCK_NONE:
		result = _transfer(send, recv, len);
		break;
	}

	return result;
}

int SPI::_transfer(uint8_t *send, uint8_t *recv, unsigned len)
{
	SPI_SETFREQUENCY(_dev, _frequency);
	SPI_SETMODE(_dev, _mode);
	SPI_SETBITS(_dev, 8);
	SPI_SELECT(_dev, _device, true);

	/* do the transfer */
	SPI_EXCHANGE(_dev, send, recv, len);

	/* and clean up */
	SPI_SELECT(_dev, _device, false);

	return 0;
}

int SPI::transferhword(uint16_t *send, uint16_t *recv, unsigned len)
{
	int result;

	if ((send == nullptr) && (recv == nullptr)) {
		return -1;
	}

	LockMode mode = up_interrupt_context() ? LOCK_NONE : _locking_mode;

	/* lock the bus as required */
	switch (mode) {
	default:
	case LOCK_PREEMPTION: {
			int state = enter_critical_section();
			result = _transferhword(send, recv, len);
			leave_critical_section(state);
		}
		break;

	case LOCK_THREADS:
		SPI_LOCK(_dev, true);
		result = _transferhword(send, recv, len);
		SPI_LOCK(_dev, false);
		break;

	case LOCK_NONE:
		result = _transferhword(send, recv, len);
		break;
	}

	return result;
}

int SPI::_transferhword(uint16_t *send, uint16_t *recv, unsigned len)
{
	SPI_SETFREQUENCY(_dev, _frequency);
	SPI_SETMODE(_dev, _mode);
	SPI_SETBITS(_dev, 16);			/* 16 bit transfer */
	SPI_SELECT(_dev, _device, true);

	/* do the transfer */
	SPI_EXCHANGE(_dev, send, recv, len);

	/* and clean up */
	SPI_SELECT(_dev, _device, false);

	return 0;
}

} // namespace device

