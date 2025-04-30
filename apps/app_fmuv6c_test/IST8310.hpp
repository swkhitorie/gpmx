/**
 * @file IST8310.hpp
 *
 * Driver for the iSentek IST8310 connected via I2C.
 *
 */

#pragma once

#include "iSentek_IST8310_registers.hpp"

#include <drivers/drv_hrt.h>
#include <device/dnode.h>
#include <device/i2c_master.h>
#include <cstdint>

using namespace iSentek_IST8310;

class IST8310
{
public:
    IST8310(const char* i2c_ops_name);
    ~IST8310();

    void run();

    int init();

    void print_status();

    float mag_gaus[3];

private:

    bool ops_valid;
    char ops_name[16];
    struct i2c_master_s *ops;

    // Sensor Configuration
    struct register_config_t {
        Register reg;
        uint8_t set_bits{0};
        uint8_t clear_bits{0};
    };

    bool register_check(const register_config_t &reg_cfg);
    uint8_t register_read(Register reg);
    void register_reads(Register reg, uint8_t *buf, uint16_t sz);
    void register_write(Register reg, uint8_t value);
    void register_modify(Register reg, uint8_t setbits, uint8_t clearbits);

    bool config();

    bool reset();


    uint8_t _addr;

    float _scale;

    uint8_t _rawmag[6];
    int16_t _mag[3];

    enum class STATE : uint8_t {
        RESET,
        WAIT_FOR_RESET,
        CONFIGURE,
        MEASURE,
        READ,
    } _state{STATE::RESET};

    uint8_t _checked_register{0};
    static constexpr uint8_t size_register_cfg{4};
    register_config_t _register_cfg[size_register_cfg] {
        // Register               | Set bits, Clear bits
        { Register::CNTL2,        0, CNTL2_BIT::SRST },
        { Register::CNTL3,        CNTL3_BIT::Z_16BIT | CNTL3_BIT::Y_16BIT | CNTL3_BIT::X_16BIT, 0 },
        { Register::AVGCNTL,      AVGCNTL_BIT::Y_16TIMES_SET | AVGCNTL_BIT::XZ_16TIMES_SET, AVGCNTL_BIT::Y_16TIMES_CLEAR | AVGCNTL_BIT::XZ_16TIMES_CLEAR },
        { Register::PDCNTL,       PDCNTL_BIT::PULSE_NORMAL, 0 },
    };
};
