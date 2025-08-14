#include "mpu6050.h"
#include <device/i2c_master.h>
#include <stdio.h>
struct i2c_master_s *bus;

static int i2c_register_write(uint8_t reg, uint8_t data)
{
    int ret;
    uint8_t cmd[2] {};
    cmd[0] = reg;
    cmd[1] = data;

    struct i2c_msg_s msg;
    msg.frequency = 100000;
    msg.addr = MPU_I2C_ADDR;
    msg.buffer = &cmd[0];
    msg.length = 2;
    msg.flags = 0;

    ret = I2C_TRANSFER(bus, &msg, 1);
    return ret;
}

static int i2c_register_read(uint8_t reg)
{
    uint8_t cmd[2] {};
    cmd[0] = reg;

    struct i2c_msg_s msg[2];
    msg[0].frequency = 100000;
    msg[0].addr = MPU_I2C_ADDR;
    msg[0].buffer = &cmd[0];
    msg[0].length = 1;
    msg[0].flags = 0;

    msg[1].frequency = 100000;
    msg[1].addr = MPU_I2C_ADDR;
    msg[1].buffer = &cmd[1];
    msg[1].length = 1;
    msg[1].flags = I2C_M_READ;

    int ret = I2C_TRANSFER(bus, msg, 2);

    return cmd[1];
}

static void i2c_register_reads(uint8_t reg, uint8_t *buf, uint16_t sz)
{
    int ret;
    uint8_t cmd;
    cmd = reg;

    struct i2c_msg_s msg[2];
    msg[0].frequency = 100000;
    msg[0].addr = MPU_I2C_ADDR;
    msg[0].buffer = &cmd;
    msg[0].length = 1;
    msg[0].flags = 0;

    msg[1].frequency = 100000;
    msg[1].addr = MPU_I2C_ADDR;
    msg[1].buffer = buf;
    msg[1].length = sz;
    msg[1].flags = I2C_M_READ;

    ret = I2C_TRANSFER(bus, msg, 2);
}

int mpu6050_init(int busn)
{
    int ret = 0;
    bus = i2c_bus_get(busn);

    if (!bus) {
        return -1;
    }

	uint8_t config[10][2] = {
        MPU_PWR_MGMT1_REG , 0x03,  
        MPU_GYRO_CFG_REG  , 0x18,
        MPU_ACCEL_CFG_REG , 0x18,
        MPU_CFG_REG		  , 0x00,
        MPU_SAMPLE_RATE_REG,0x00,

        //Enable bypass mode
        MPU_USER_CTRL_REG , 0x00,
        MPU_INTBP_CFG_REG , 0x02
    };

    for (int i = 0; i < 7; i++) {
        ret = i2c_register_write(config[i][0], config[i][1]);
    }

    for (int i = 0; i < 7; i++) {
        ret = i2c_register_read(config[i][0]);
        if (ret != config[i][1]) {
            return -2;
        }
    }

    return 0;
}

int mpu6050_read(float *data)
{
    int ret = 0;
    uint8_t buf[14];
    uint8_t reg = MPU_ACCEL_XOUTH_REG;

    int16_t accel_x, accel_y, accel_z;
    int16_t gyro_x, gyro_y, gyro_z;

    float accx, accy, accz;
    float gyrox, gyroy, gyroz;

    i2c_register_reads(reg, &buf[0], 14);

	accel_x = (int16_t)(buf[0]<<8 | buf[1]);
	accel_y= (int16_t)(buf[2]<<8 | buf[3]);
	accel_z = (int16_t)(buf[4]<<8 | buf[5]);
	gyro_x = (int16_t)(buf[8]<<8 | buf[9]);
	gyro_y = (int16_t)(buf[10]<<8 | buf[11]);
	gyro_z = (int16_t)(buf[12]<<8 | buf[13]);

    gyrox = (float)gyro_x * Gyro_2000An_SCALE;
    gyroy = (float)gyro_y * Gyro_2000An_SCALE;
    gyroz = (float)gyro_z * Gyro_2000An_SCALE;

    accx = (float)accel_x * ACC_16G_SCALE;
    accy = (float)accel_y * ACC_16G_SCALE;
    accz = (float)accel_z * ACC_16G_SCALE;

    data[0] = accx;
    data[1] = accy;
    data[2] = accz;

    data[3] = gyrox;
    data[4] = gyroy;
    data[5] = gyroz;

    return ret;
}
