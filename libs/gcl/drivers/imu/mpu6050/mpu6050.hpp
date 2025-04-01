#pragma once

#include "mpu6050_register.hpp"
#include <drivers/i2c.h>
#include <cstdint>

using namespace InvenSense_MPU6050;

class MPU6050
{
public:
    MPU6050(const char* i2c_ops);
    ~MPU6050() override;

    static void print_usage();

    void run();

    int init();

private:

    struct i2c_master_s *ops;

    struct register_config_t {
        Register reg;
        uint8_t set_bits{0};
        uint8_t clear_bits{0};
    };

    bool register_check(const register_config_t &reg_cfg);
    uint8_t register_read(Register reg);
    void register_write(Register reg, uint8_t value);
    void register_modify(Register reg, uint8_t setbits, uint8_t clearbits);

    bool config();
    void config_accel();
    void config_gyro();
    void config_samplerate(int sample_rate);

    // Sensor Configuration
    static constexpr float FIFO_SAMPLE_DT{1e6f / 8000.f};
    static constexpr int32_t SAMPLES_PER_TRANSFER{8};                    // ensure at least 1 new accel sample per transfer
    static constexpr float GYRO_RATE{1e6f / FIFO_SAMPLE_DT};             // 8000 Hz gyro
    static constexpr float ACCEL_RATE{GYRO_RATE / SAMPLES_PER_TRANSFER}; // 1000 Hz accel


    int probe() override;

    bool Reset();

    uint16_t FIFOReadCount();
    bool FIFORead(const hrt_abstime &timestamp_sample, uint8_t samples);
    void FIFOReset();

    bool ProcessAccel(const hrt_abstime &timestamp_sample, const FIFO::DATA fifo[], const uint8_t samples);
    void ProcessGyro(const hrt_abstime &timestamp_sample, const FIFO::DATA fifo[], const uint8_t samples);
    void UpdateTemperature();

    hrt_abstime _reset_timestamp{0};
    hrt_abstime _last_config_check_timestamp{0};
    hrt_abstime _temperature_update_timestamp{0};

    enum class STATE : uint8_t {
        RESET,
        WAIT_FOR_RESET,
        CONFIGURE,
        READ,
    } _state{STATE::RESET};

    uint8_t _checked_register{0};
    static constexpr uint8_t size_register_cfg{3};
    register_config_t _register_cfg[size_register_cfg] {
        // Register               | Set bits, Clear bits
        { Register::GYRO_CONFIG,   GYRO_CONFIG_BIT::FS_SEL_2000_DPS, 0 },
        { Register::ACCEL_CONFIG,  ACCEL_CONFIG_BIT::AFS_SEL_16G, 0 },
        { Register::PWR_MGMT_1,    PWR_MGMT_1_BIT::CLKSEL_0, PWR_MGMT_1_BIT::SLEEP },
    };

};
