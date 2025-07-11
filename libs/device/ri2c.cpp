#include <device/ri2c.hpp>


namespace device
{

RI2C::RI2C(const char *name, const char *bus, const uint16_t address, const uint32_t frequency) :
	_frequency(frequency),
	_address(address)
{
    strcpy(const_cast<char *>(&_devname[0]), name);
    strcpy(const_cast<char *>(&_busname[0]), bus);
}

RI2C::~RI2C()
{
	if (_dev) {
		_dev = nullptr;
	}
}

int
RI2C::init()
{
	int ret = 0;

	// attach to the i2c bus
    _dev = static_cast<struct i2c_master_s *>(dbind(_busname));

	if (_dev == nullptr) {
		DEVICE_DEBUG("failed to init I2C");
		ret = DEV_NOINIT;
		goto out;
	}

	// call the probe function to check whether the device is present
	ret = probe();

	if (ret != DEV_OK) {
		goto out;
	}

out:

	if ((ret != DEV_OK) && (_dev != nullptr)) {
		_dev = nullptr;
	}

	return ret;
}

int
RI2C::transfer(const uint8_t regsz, const uint8_t *send, const unsigned send_len, uint8_t *recv, const unsigned recv_len)
{
	int ret = DEV_ERROR;

	if (_dev == nullptr) {
		return DEV_ERROR;
	}

	do {

        struct i2c_msg_s msgv;

        if (send_len <= 0) {
            // do nothing
            ret = DEV_OK;
            break;
        }

		if (recv_len <= 0) {
            msgv.flags = I2C_REG_WRITE;
			msgv.addr = _address << 1;
            msgv.reg_sz = regsz;
			msgv.flags = 0;
			msgv.xbuffer = const_cast<uint8_t *>(send);
			msgv.xlength = send_len;
		} else {
            msgv.flags = I2C_REG_READ;
			msgv.addr = _address << 1;
            msgv.reg_sz = regsz;

			msgv.xbuffer = const_cast<uint8_t *>(send);
			msgv.xlength = send_len;

			msgv.rbuffer = const_cast<uint8_t *>(recv);
			msgv.xlength = recv_len;
        }

		int ret_transfer = I2C_TRANSFER(_dev, &msgv, 1);

		if (ret_transfer != 0) {
			ret = DEV_ERROR;
		} else {
			ret = DEV_OK;
		}

	} while (0);

	return ret;
}

} // namespace device
