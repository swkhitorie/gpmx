#include "hmc5883l.hpp"
#include <string.h>
#include <stdio.h>

static constexpr int16_t combine(uint8_t msb, uint8_t lsb)
{
    return (msb << 8u) | lsb;
}

static void delayforworks(uint64_t time_us)
{
    hrt_abstime now = hrt_absolute_time();
    while ((hrt_absolute_time() - now) < time_us);
}

HMC5883::HMC5883(const char* i2c_ops_name)
{
    _lst_update_time = 0;
    strcpy(&ops_name[0], i2c_ops_name);
}
HMC5883::~HMC5883() {}

int HMC5883::init()
{
    ops = static_cast<struct i2c_master_s*>(dbind(ops_name));
    ops_valid = (ops != nullptr);

    if (ops_valid) {
        register_reads(HMC5883_register::ID_A, &id_check[0], 3);
        if (id_check[0] != ID_A_WHO_AM_I ||
            id_check[1] != ID_B_WHO_AM_I ||
            id_check[2] != ID_C_WHO_AM_I) {
            ops_valid = false;
        }
    }

    if (!ops_valid) return false;

    reset();
    return config();
}

void HMC5883::run()
{
    if (hrt_absolute_time() - _lst_update_time < 0.02 * 1e6) {
        return;
    }

    _lst_update_time = hrt_absolute_time();
    process_data();
}

bool HMC5883::register_check(const register_config_t &reg_cfg)
{
    bool success = true;

    const uint8_t reg_value = register_read(reg_cfg.reg);

    if (reg_cfg.set_bits && ((reg_value & reg_cfg.set_bits) != reg_cfg.set_bits)) {
        success = false;
    }

    if (reg_cfg.clear_bits && ((reg_value & reg_cfg.clear_bits) != 0)) {
        success = false;
    }

    return success;
}

uint8_t HMC5883::register_read(HMC5883_register reg)
{
    uint8_t cmd[2] {};
    cmd[0] = static_cast<uint8_t>(reg);

    struct i2c_msg_s rmsg;
    rmsg.flags = I2C_REG_READ;
    rmsg.addr = IIC_ADDR<<1; rmsg.reg_sz = 1;
    rmsg.xbuffer = &cmd[0];
    rmsg.xlength = 1;
    rmsg.rbuffer = &cmd[1];
    rmsg.rlength = 1;
    int ret = I2C_TRANSFER(ops, &rmsg, 1);

    return cmd[1];
}

void HMC5883::register_reads(HMC5883_register reg, uint8_t *buf, uint16_t sz)
{
    int ret;
    uint8_t cmdad;
    cmdad = static_cast<uint8_t>(reg);

    struct i2c_msg_s rxmsg;
    rxmsg.flags = I2C_REG_READ;
    rxmsg.addr = IIC_ADDR<<1; rxmsg.reg_sz = 1;
    rxmsg.xbuffer = &cmdad;
    rxmsg.xlength = 1;
    rxmsg.rbuffer = buf;
    rxmsg.rlength = sz;
    ret = I2C_TRANSFER(ops, &rxmsg, 1);
}

void HMC5883::register_write(HMC5883_register reg, uint8_t value)
{
    int ret;
    uint8_t cmd[2] {};
    cmd[0] = static_cast<uint8_t>(reg);
    cmd[1] = value;

    struct i2c_msg_s wmsg;
    wmsg.flags = I2C_REG_WRITE;
    wmsg.addr = IIC_ADDR<<1;
    wmsg.reg_sz = 1;
    wmsg.xbuffer = &cmd[0];
    wmsg.xlength = 2;
    ret = I2C_TRANSFER(ops, &wmsg, 1);
}

void HMC5883::register_modify(HMC5883_register reg, uint8_t setbits, uint8_t clearbits)
{
    const uint8_t orig_val = register_read(reg);
    const uint8_t tr_origin = 0x00;
    uint8_t val = (tr_origin & ~clearbits) | setbits;

    if (orig_val != val) {
        register_write(reg, val);
    }
}

bool HMC5883::reset()
{
    return true;
}

bool HMC5883::config()
{
	// first set and clear all configured register bits
	for (const auto &reg_cfg : _register_cfg) {
		register_modify(reg_cfg.reg, reg_cfg.set_bits, reg_cfg.clear_bits);
	}

	// now check that all are configured
	bool success = true;

	for (const auto &reg_cfg : _register_cfg) {
		if (!register_check(reg_cfg)) {
			success = false;
		}
	}
    return success;
}

void HMC5883::process_data()
{
    register_reads(HMC5883_register::DATA_OUT_X_MSB, &rawmag[0], 6);

    mag[0] = combine(rawmag[0], rawmag[1]);
    mag[1] = combine(rawmag[2], rawmag[3]);
    mag[2] = combine(rawmag[4], rawmag[5]);

    for (int i = 0; i < 3; i++) {
        mag_gaus[i] = mag[i];// * 1000.0f / HMC5883L_GA_LSB2;
    }
}
