#ifndef MPU6050_I2C_H_
#define MPU6050_I2C_H_

#include <dev/dnode.h>
#include <dev/i2c_master.h>
#include <drivers/drv_hrt.h>
#include <cstdint>

enum Mpu6050_register {
    SELF_TESTX = 0X0D,
    SELF_TESTY = 0X0E,
    SELF_TESTZ = 0X0F,
    SELF_TESTA = 0X10,
    SAMPLE_RATE	= 0X19,
    CFG = 0X1A,
    GYRO_CFG = 0X1B,
    ACCEL_CFG = 0X1C,
    ACCEL_CFG2 = 0X1D,
    MOTION_DET = 0X1F,
    FIFO_EN = 0X23,
    I2CMST_CTRL = 0X24,
    I2CSLV0_ADDR = 0X25,
    I2CSLV0 = 0X26,
    I2CSLV0_CTRL = 0X27,
    I2CSLV1_ADDR = 0X28,
    I2CSLV1 = 0X29,
    I2CSLV1_CTRL = 0X2A,
    I2CSLV2_ADDR = 0X2B,
    I2CSLV2 = 0X2C,
    I2CSLV2_CTRL = 0X2D,
    I2CSLV3_ADDR = 0X2E,
    I2CSLV3 = 0X2F,
    I2CSLV3_CTRL = 0X30,
    I2CSLV4_ADDR = 0X31,
    I2CSLV4 = 0X32,
    I2CSLV4_DO = 0X33,
    I2CSLV4_CTRL = 0X34,
    I2CSLV4_DI = 0X35,
    I2CMST_STA = 0X36,
    INTBP_CFG = 0X37,
    INT_EN = 0X38,
    INT_STA = 0X3A,
    ACCEL_XOUTH = 0X3B,
    ACCEL_XOUTL = 0X3C,
    ACCEL_YOUTH = 0X3D,
    ACCEL_YOUTL = 0X3E,
    ACCEL_ZOUTH = 0X3F,
    ACCEL_ZOUTL = 0X40,
    TEMP_OUTH = 0X41,
    TEMP_OUTL = 0X42,
    GYRO_XOUTH = 0X43,
    GYRO_XOUTL = 0X44,
    GYRO_YOUTH = 0X45,
    GYRO_YOUTL = 0X46,
    GYRO_ZOUTH = 0X47,
    GYRO_ZOUTL = 0X48,

    I2CSLV0_DO = 0X63,
    I2CSLV1_DO = 0X64,
    I2CSLV2_DO = 0X65,
    I2CSLV3_DO = 0X66,
    I2CMST_DELAY = 0X67,
    SIGPATH_RST = 0X68,
    MDETECT_CTRL = 0X69,
    USER_CTRL = 0X6A,
    PWR_MGMT1 = 0X6B,
    PWR_MGMT2 = 0X6C,
    FIFO_CNTH = 0X72,
    FIFO_CNTL = 0X73,
    FIFO_RW = 0X74,
    DEVICE_ID = 0X75,
};

enum Mpu6050_id {
    ID_1 = 0x68,
    ID_2 = 0x71,
    ID_3 = 0x73,
    ID_4 = 0x70,
};

class MPU6050 {

public:
    MPU6050(const char* i2c_ops_name);
    ~MPU6050();

    float gyro_deg[3];
    float accel_g[3];

    void run();

    int init();

private:
    bool ops_valid;
    char ops_name[16];
    struct i2c_master_s *ops;

    static constexpr uint8_t IIC_ADDR = 0x68;
    static constexpr float ACC_2G_SCALE = static_cast<float>((2 * 2) / 65536.0);
    static constexpr float ACC_4G_SCALE = static_cast<float>((2 * 4) / 65536.0);
    static constexpr float ACC_8G_SCALE = static_cast<float>((2 * 8) / 65536.0);
    static constexpr float ACC_16G_SCALE = static_cast<float>((2 * 16) / 65536.0);
    static constexpr float Gyro_250An_SCALE = static_cast<float>((2 * 250.0) / 65536.0);
    static constexpr float Gyro_500An_SCALE = static_cast<float>((2 * 500.0) / 65536.0);
    static constexpr float Gyro_1000An_SCALE = static_cast<float>((2 * 1000.0) / 65536.0);
    static constexpr float Gyro_2000An_SCALE = static_cast<float>((2 * 2000.0) / 65536.0);

    struct register_config_t {
        Mpu6050_register reg;
        uint8_t set_bits{0};
        uint8_t clear_bits{0};
    };

    bool register_check(const register_config_t &reg_cfg);
    uint8_t register_read(Mpu6050_register reg);
    void register_reads(Mpu6050_register reg, uint8_t *buf, uint16_t sz);
    void register_write(Mpu6050_register reg, uint8_t value);
    void register_modify(Mpu6050_register reg, uint8_t setbits, uint8_t clearbits);

    bool reset();

    bool config();

    void process_data();

	uint8_t rawgyro[6];
	uint8_t rawtemp[2];
	uint8_t rawacc[6];
	uint8_t rawall[14];
	
	int16_t gyro_origin[3];
	int16_t acc_origin[3];
    int16_t temper_origin;

    static constexpr uint8_t size_register_cfg{5};
    register_config_t _register_cfg[size_register_cfg] {
        // Register                 | Set bits, Clear bits
        { Mpu6050_register::ACCEL_CFG,      3<<3,   0 },   /* ±16g */
        { Mpu6050_register::GYRO_CFG,       3<<3,   0 },   /* ±2000dps */
        { Mpu6050_register::SAMPLE_RATE,    0,      0 },   /* max rate */
        { Mpu6050_register::CFG,            0x07,   0 },   /* no DLPF setting */
        { Mpu6050_register::INTBP_CFG,      0x02,   0 },   /* enable i2c bypass for other sensor */
    };
};

#endif
