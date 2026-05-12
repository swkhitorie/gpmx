#include "i2cbus.hpp"
#include "board_config.h"
namespace device
{

unsigned int I2C::_bus_clocks[NUMBER_I2C_BUSES] = {400000};

I2C::I2C(const char *name, const int bus, const uint16_t address, const uint32_t frequency) :
	_devname(name),
	_bus(bus),
    _address(address),
    _frequency(frequency)
{
	_bus_clocks[_bus - 1] = _frequency;
}

I2C::~I2C()
{
    
}

int I2C::set_bus_clock(unsigned bus, unsigned clock_hz)
{
	int index = bus - 1;

	if (index < 0 || index >= static_cast<int>(sizeof(_bus_clocks) / sizeof(_bus_clocks[0]))) {
		return -1;
	}

	_bus_clocks[index] = clock_hz;

	return 0;
}

int I2C::init()
{
	int ret = -1;
	unsigned bus_index;

	// attach to the i2c bus
    _dev = i2c_bus_get(_bus);

	if (_dev == nullptr) {
		ret = -2;
		return ret;
	}

	bus_index = _bus - 1;

	// abort if the max frequency we allow (the frequency we ask)
	// is smaller than the bus frequency
	if (_bus_clocks[bus_index] > _frequency) {
		_dev = nullptr;
		ret = -3;
		return ret;
	}

	if (_bus_clocks[bus_index] == 0) {
		_bus_clocks[bus_index] = _frequency;
	}

	// call the probe function to check whether the device is present
	ret = probe();
	if (ret != 0) {
		return -4;
	}

	return ret;
}

int I2C::transfer(const uint8_t *send, const unsigned send_len, uint8_t *recv, const unsigned recv_len)
{
	int ret = -1;
	unsigned retry_count = 0;

	if (_dev == nullptr) {
		return -2;
	}

	do {

		i2c_msg_s msgv[2] {};
		unsigned msgs = 0;

		if (send_len > 0) {
			msgv[msgs].frequency = _bus_clocks[_bus - 1];
			msgv[msgs].addr = _address;
			msgv[msgs].flags = 0;
			msgv[msgs].buffer = const_cast<uint8_t *>(send);
			msgv[msgs].length = send_len;
			msgs++;
		}

		if (recv_len > 0) {
			msgv[msgs].frequency = _bus_clocks[_bus - 1];
			msgv[msgs].addr = _address;
			msgv[msgs].flags = I2C_M_READ;
			msgv[msgs].buffer = recv;
			msgv[msgs].length = recv_len;
			msgs++;
		}

		if (msgs == 0) {
			return -3;
		}

		int ret_transfer = I2C_TRANSFER(_dev, &msgv[0], msgs);

		if (ret_transfer != 0) {
			ret = -4;

		} else {
			// success
			ret = 0;
			break;
		}

		// if we have already retried once, and we aren't going to give up, then reset the bus
		if ((_retries > 0) && (retry_count < _retries)) {
			// I2C_RESET(_dev);
		}

	} while (retry_count++ < _retries);

	return ret;
}

} // namespace device

