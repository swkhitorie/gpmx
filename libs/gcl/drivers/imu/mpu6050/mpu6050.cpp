#include "mpu6050.hpp"
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

MPU6050::MPU6050(const char* i2c_ops_name)
{
    strcpy(&ops_name[0], i2c_ops_name);
}
MPU6050::~MPU6050() {}

int MPU6050::init()
{
    ops = static_cast<struct i2c_master_s*>(dbind(ops_name));
    ops_valid = (ops != nullptr);

    uint8_t id;
    if (ops_valid) {
        id = register_read(Mpu6050_register::DEVICE_ID);
        if (id != ID_1 && id != ID_2 && id != ID_3 && id != ID_4) {
            ops_valid = false;
        }
    }

    if (!ops_valid) return false;

    reset();
    return config();
}

void MPU6050::run()
{
    process_data();
}

bool MPU6050::register_check(const register_config_t &reg_cfg)
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

uint8_t MPU6050::register_read(Mpu6050_register reg)
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

void MPU6050::register_reads(Mpu6050_register reg, uint8_t *buf, uint16_t sz)
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

void MPU6050::register_write(Mpu6050_register reg, uint8_t value)
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

void MPU6050::register_modify(Mpu6050_register reg, uint8_t setbits, uint8_t clearbits)
{
    const uint8_t orig_val = register_read(reg);

    uint8_t val = (orig_val & ~clearbits) | setbits;

    if (orig_val != val) {
        register_write(reg, val);
    }
}

bool MPU6050::reset()
{
    /* reset device */
    register_write(Mpu6050_register::PWR_MGMT1, 0x80);
    delayforworks((1e6f / 100));

    /* device clock select: PLL with X gyro */
    register_write(Mpu6050_register::PWR_MGMT1, 0x01);
    delayforworks((1e6f / 100));

    /* reset gyro, accel, temperature analog signal paths */
    register_write(Mpu6050_register::SIGPATH_RST, 0x07);
    delayforworks((1e6f / 100));

    /* disable fifo, i2c master */
    register_write(Mpu6050_register::USER_CTRL, 0x00);
    delayforworks((1e6f / 100));

    /* enable all sensors into standby mode */
    register_write(Mpu6050_register::PWR_MGMT2, 0x00);
    delayforworks((1e6f / 100));

    return true;
}

bool MPU6050::config()
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

void MPU6050::process_data()
{
    register_reads(Mpu6050_register::ACCEL_XOUTH, &rawall[0], 14);

	rawacc[0] = rawall[0];
	rawacc[1] = rawall[1];
	rawacc[2] = rawall[2];
	rawacc[3] = rawall[3];
	rawacc[4] = rawall[4];
	rawacc[5] = rawall[5];

	rawtemp[0] = rawall[6];
	rawtemp[1] = rawall[7];

	rawgyro[0] = rawall[8];
	rawgyro[1] = rawall[9];
	rawgyro[2] = rawall[10];
	rawgyro[3] = rawall[11];
	rawgyro[4] = rawall[12];
	rawgyro[5] = rawall[13];	

    gyro_origin[0] = combine(rawgyro[0], rawgyro[1]);
    gyro_origin[1] = combine(rawgyro[2], rawgyro[3]);
    gyro_origin[2] = combine(rawgyro[4], rawgyro[5]);
    
    acc_origin[0] = combine(rawacc[0], rawacc[1]);
    acc_origin[1] = combine(rawacc[2], rawacc[3]);
    acc_origin[2] = combine(rawacc[4], rawacc[5]);
    
	temper_origin = combine(rawtemp[0], rawtemp[1]);
    for (int i = 0; i < 3; i++) {
        gyro_deg[i] = gyro_origin[i] * Gyro_2000An_SCALE;
        accel_g[i] =  acc_origin[i] * ACC_16G_SCALE;
    }
}
