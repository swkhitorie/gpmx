#include <device/spi.hpp>

namespace device
{

SPI::SPI(const char *name, const char *bus, uint32_t device, enum spi_mode_e mode, uint32_t frequency) :
	_device(device),
	_mode(mode),
	_frequency(frequency)
{
    strcpy(_devname, name, strlen(name));
    strcpy(_busname, bus, strlen(bus));
}

SPI::~SPI()
{
	// do nothing
}

int
SPI::init()
{
	/* attach to the spi bus */
	if (_dev == nullptr) {
        _dev = static_cast<struct spi_dev_s *>(dbind(_busname));
	}

	if (_dev == nullptr) {
		return DEV_ERROR;
	}

	/* call the probe function to check whether the device is present */
	int ret = probe();

	if (ret != DEV_OK) {
		return ret;
	}

	return DEV_OK;
}

int
SPI::transfer(uint8_t *send, uint8_t *recv, unsigned len)
{
	int result;

	if ((send == nullptr) && (recv == nullptr)) {
		return DEV_ERROR;
	}

	LockMode mode = _locking_mode;

	/* lock the bus as required */
	switch (mode) {
	default:
	case LOCK_PREEMPTION: {
			UBaseType_t	flags = portSET_INTERRUPT_MASK_FROM_ISR();
			result = _transfer(send, recv, len);
			portCLEAR_INTERRUPT_MASK_FROM_ISR(flags);
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

int
SPI::_transfer(uint8_t *send, uint8_t *recv, unsigned len)
{
	SPI_SETFREQUENCY(_dev, _frequency);
	SPI_SETMODE(_dev, _mode);
	SPI_SETBITS(_dev, 8);
	SPI_SELECT(_dev, _device, true);

	/* do the transfer */
	SPI_EXCHANGE(_dev, send, recv, len);

	/* and clean up */
	SPI_SELECT(_dev, _device, false);

	return DEV_OK;
}

} // namespace device
