#include "IST8310.hpp"
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

IST8310::IST8310(const char* i2c_ops_name)
{
    strcpy(&ops_name[0], i2c_ops_name);
    _addr = I2C_ADDRESS_DEFAULT;
}
IST8310::~IST8310() {}

bool IST8310::register_check(const register_config_t &reg_cfg)
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

uint8_t IST8310::register_read(Register reg)
{
    uint8_t cmd[2] {};
    cmd[0] = static_cast<uint8_t>(reg);

    struct i2c_msg_s rmsg;
    rmsg.flags = I2C_REG_READ;
    rmsg.addr = _addr<<1; rmsg.reg_sz = 1;
    rmsg.xbuffer = &cmd[0];
    rmsg.xlength = 1;
    rmsg.rbuffer = &cmd[1];
    rmsg.rlength = 1;
    int ret = I2C_TRANSFER(ops, &rmsg, 1);

    return cmd[1];
}

void IST8310::register_reads(Register reg, uint8_t *buf, uint16_t sz)
{
    int ret;
    uint8_t cmdad;
    cmdad = static_cast<uint8_t>(reg);

    struct i2c_msg_s rxmsg;
    rxmsg.flags = I2C_REG_READ;
    rxmsg.addr = _addr<<1; rxmsg.reg_sz = 1;
    rxmsg.xbuffer = &cmdad;
    rxmsg.xlength = 1;
    rxmsg.rbuffer = buf;
    rxmsg.rlength = sz;
    ret = I2C_TRANSFER(ops, &rxmsg, 1);
}

void IST8310::register_write(Register reg, uint8_t value)
{
    int ret;
    uint8_t cmd[2] {};
    cmd[0] = static_cast<uint8_t>(reg);
    cmd[1] = value;

    struct i2c_msg_s wmsg;
    wmsg.flags = I2C_REG_WRITE;
    wmsg.addr = _addr<<1;
    wmsg.reg_sz = 1;
    wmsg.xbuffer = &cmd[0];
    wmsg.xlength = 2;
    ret = I2C_TRANSFER(ops, &wmsg, 1);
}

void IST8310::register_modify(Register reg, uint8_t setbits, uint8_t clearbits)
{
    const uint8_t orig_val = register_read(reg);
    const uint8_t tr_origin = 0x00;
    uint8_t val = (tr_origin & ~clearbits) | setbits;

    if (orig_val != val) {
        register_write(reg, val);
    }
}

bool IST8310::config()
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

    _scale = 1.f / 1320.f; // 1320 LSB/Gauss

    return success;
}

bool IST8310::reset()
{
    // CNTL2: Software Reset
    register_write(Register::CNTL2, CNTL2_BIT::SRST);
    delayforworks(100*1000);

    return true;
}

int IST8310::init()
{
    const auto start_time = hrt_absolute_time();

    ops = static_cast<struct i2c_master_s*>(dbind(ops_name));
    ops_valid = (ops != nullptr);

    if (!ops_valid) return false;

    // Reading the WAI register is not always reliable, it can return 0xff or
    // other values if the sensor has been powered up in a certain way. In
    // addition, the I2C address is not always correct, sometimes it boots with
    // 0x0C rather than 0x0E.
    _addr = I2C_ADDRESS_DEFAULT;

    reset();

    const int WAI = register_read(Register::WAI);

    if (WAI != Device_ID) return false;

    return config();
}

void IST8310::run()
{
    register_write(Register::CNTL1, CNTL1_BIT::MODE_SINGLE_MEASUREMENT);

    delayforworks(20'000);

    register_reads(Register::DATAXL, &_rawmag[0], 6);

    _mag[0] = combine(_rawmag[1], _rawmag[0]);
    _mag[1] = combine(_rawmag[3], _rawmag[2]);
    _mag[2] = combine(_rawmag[5], _rawmag[4]);

    mag_gaus[0] = _mag[0] * _scale;
    mag_gaus[1] = _mag[1] * _scale;
    mag_gaus[2] = _mag[2] * _scale;
}

void IST8310::print_status()
{
    printf("[ist8310] x: %.3f, %.3f, %.3f \r\n", mag_gaus[0], mag_gaus[1], mag_gaus[2]);
}
