/**
 * @file IST8310.hpp
 *
 * Driver for the iSentek IST8310 connected via I2C.
 *
 */

#pragma once

#include "iSentek_IST8310_registers.hpp"

#include <drivers/drv_hrt.h>
#include <dev/dnode.h>
#include <dev/i2c_master.h>
#include <cstdint>

using namespace iSentek_IST8310;

class IST8310
{
public:
    IST8310(const I2CSPIDriverConfig &config);
    ~IST8310();

    static void print_usage();

    void RunImpl();

    int init();
    void print_status();

private:
    // Sensor Configuration
    struct register_config_t {
        Register reg;
        uint8_t set_bits{0};
        uint8_t clear_bits{0};
    };

    bool Reset();

    bool Configure();

    bool RegisterCheck(const register_config_t &reg_cfg);

    int RegisterRead(Register reg);
    void RegisterWrite(Register reg, uint8_t value);
    void RegisterSetAndClearBits(Register reg, uint8_t setbits, uint8_t clearbits);

    hrt_abstime _reset_timestamp{0};
    hrt_abstime _last_config_check_timestamp{0};
    int _failure_count{0};

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
