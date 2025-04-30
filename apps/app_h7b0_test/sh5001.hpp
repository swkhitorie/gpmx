#ifndef SH5001_H_
#define SH5001_H_

#include <device/dnode.h>
#include <device/i2c_master.h>
#include <drivers/drv_hrt.h>
#include <cstdint>

#include "sh5001_registers.hpp"

class SH5001
{
public:
    SH5001(const char* i2c_ops_name);
    ~SH5001();

    float gyro_rad[3];
    float accel_m2_s[3];

    void run();

    int init();

public:
    bool ops_valid;
    char ops_name[16];
    struct i2c_master_s *ops;

    static constexpr float ACC_2G_SCALE = static_cast<float>((2 * 2) / 65536.0);
    static constexpr float ACC_4G_SCALE = static_cast<float>((2 * 4) / 65536.0);
    static constexpr float ACC_8G_SCALE = static_cast<float>((2 * 8) / 65536.0);
    static constexpr float ACC_16G_SCALE = static_cast<float>((2 * 16) / 65536.0);
    static constexpr float Gyro_250An_SCALE = static_cast<float>((2 * 250.0) / 65536.0);
    static constexpr float Gyro_500An_SCALE = static_cast<float>((2 * 500.0) / 65536.0);
    static constexpr float Gyro_1000An_SCALE = static_cast<float>((2 * 1000.0) / 65536.0);
    static constexpr float Gyro_2000An_SCALE = static_cast<float>((2 * 2000.0) / 65536.0);

    struct register_config_t {
        uint8_t reg;
        uint8_t set_bits{0};
        uint8_t clear_bits{0};
    };

    bool register_check(const register_config_t &reg_cfg);
    uint8_t register_read(uint8_t reg);
    void register_reads(uint8_t reg, uint8_t *buf, uint16_t sz);
    void register_write(uint8_t reg, uint8_t value);
    void register_modify(uint8_t reg, uint8_t setbits, uint8_t clearbits);

	void soft_reset();
	void drive_start();	
	void adc_reset();
	void cva_reset();
    void acc_reset();
    // reset internal module, don't modify it
    void reset();

    void osc_freq();

    void config_imu_data_stable();
    bool config();

    void get_imu();
    void get_accel();
    void get_gyro();

    int16_t accel_origin[3];
    int16_t gyro_origin[3];

    static constexpr uint8_t size_register_cfg{5};
    register_config_t _register_cfg[size_register_cfg] {
        // Register              | Set bits, Clear bits
        { SH5001_ACC_CONF1,     SH5001_ACC_ODR_500HZ_440HZ | 
                                SH5001_ACC_RANGE_16G,   0 },

        { SH5001_ACC_CONF2,     SH5001_ACC_ODRX010,      0 },   /* cutoff hz: 50hz */

        { SH5001_GYRO_CONF1,    SH5001_GYRO_ODR_125HZ_110HZ |
                                SH5001_GYRO_RANGE_2000,   0 },

        { SH5001_GYRO_CONF2,    SH5001_GYRO_ODRX040,   0 },

        { SH5001_TEMP_CONF0,    SH5001_TEMP_ODR_63HZ |
                                SH5001_TEMP_EN,        0 },
    };

};



#endif
