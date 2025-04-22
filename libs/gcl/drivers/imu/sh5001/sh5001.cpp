#include "sh5001.hpp"
#include <string.h>
#include <stdio.h>

static void delayforworks(uint64_t time_us)
{
    hrt_abstime now = hrt_absolute_time();
    while ((hrt_absolute_time() - now) < time_us);
}

SH5001::SH5001(const char* i2c_ops_name)
{
    strcpy(&ops_name[0], i2c_ops_name);
}
SH5001::~SH5001() {}

bool SH5001::register_check(const register_config_t &reg_cfg)
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

uint8_t SH5001::register_read(uint8_t reg)
{
    uint8_t cmd[2] {};
    cmd[0] = static_cast<uint8_t>(reg);

    struct i2c_msg_s rmsg;
    rmsg.flags = I2C_REG_READ;
    rmsg.addr = SH5001_ADDRESS<<1; rmsg.reg_sz = 1;
    rmsg.xbuffer = &cmd[0];
    rmsg.xlength = 1;
    rmsg.rbuffer = &cmd[1];
    rmsg.rlength = 1;
    int ret = I2C_TRANSFER(ops, &rmsg, 1);

    return cmd[1];
}

void SH5001::register_reads(uint8_t reg, uint8_t *buf, uint16_t sz)
{
    int ret;
    uint8_t cmdad;
    cmdad = static_cast<uint8_t>(reg);

    struct i2c_msg_s rxmsg;
    rxmsg.flags = I2C_REG_READ;
    rxmsg.addr = SH5001_ADDRESS<<1; rxmsg.reg_sz = 1;
    rxmsg.xbuffer = &cmdad;
    rxmsg.xlength = 1;
    rxmsg.rbuffer = buf;
    rxmsg.rlength = sz;
    ret = I2C_TRANSFER(ops, &rxmsg, 1);
}

void SH5001::register_write(uint8_t reg, uint8_t value)
{
    int ret;
    uint8_t cmd[2] {};
    cmd[0] = static_cast<uint8_t>(reg);
    cmd[1] = value;

    struct i2c_msg_s wmsg;
    wmsg.flags = I2C_REG_WRITE;
    wmsg.addr = SH5001_ADDRESS<<1;
    wmsg.reg_sz = 1;
    wmsg.xbuffer = &cmd[0];
    wmsg.xlength = 2;
    ret = I2C_TRANSFER(ops, &wmsg, 1);
}

void SH5001::register_modify(uint8_t reg, uint8_t setbits, uint8_t clearbits)
{
    const uint8_t orig_val = register_read(reg);

    uint8_t val = (orig_val & ~clearbits) | setbits;

    if (orig_val != val) {
        register_write(reg, val);
    }
}

void SH5001::soft_reset()
{
	unsigned int reg_data = 0;	
	// soft reset
	reg_data = 0x01;
	register_write(0x2B, reg_data);
	reg_data = 0x73;
	register_write(0x00, reg_data);

	delayforworks(50*1000);
}

void SH5001::drive_start()
{
    unsigned int reg_data = 0x0;
	
    reg_data = 0x01;
    register_write(0x2B, reg_data);
    delayforworks(2*1000);

    reg_data = 0x00;
    register_write(0x2B, reg_data);
    delayforworks(1*1000);
}

void SH5001::adc_reset()
{
    unsigned char reg_data = 0;

	reg_data = 0x08;
	register_write(0x30, reg_data);

	reg_data = 0x00;
	register_write(0xD2, reg_data);

	reg_data = 0x6B;
	register_write(0xD1, reg_data);

	reg_data = 0x02;
	register_write(0xD5, reg_data);
    delayforworks(2*1000);

	reg_data = 0x68;
	register_write(0xD1, reg_data);
    delayforworks(2*1000);

	reg_data = 0x00;
	register_write(0xD5, reg_data);

	reg_data = 0x00;
	register_write(0x30, reg_data);
    delayforworks(50*1000);
}

void SH5001::cva_reset()
{
	unsigned char regData = 0;
	unsigned int regDEData = 0;

	regDEData = register_read(0xDE);

	regData = regDEData & 0xC7;
	register_write(0xDE, regData);
	delayforworks(5*1000);

	regData = regDEData | 0x38;
	register_write(0xDE, regData);
	delayforworks(5*1000);

	register_write(0xDE, regDEData);
	delayforworks(5*1000);

	regData = 0x12;
	register_write(0xCD, regData);
	regData = 0x12;
	register_write(0xCE, regData);
	regData = 0x12;
	register_write(0xCF, regData);

	delayforworks(1*1000);

	regData = 0x2;
	register_write(0xCD, regData);
	regData = 0x2;
	register_write(0xCE, regData);
	regData = 0x2;
	register_write(0xCF, regData);
}

void SH5001::acc_reset()
{
    unsigned char regData = 0;

	regData = 0x08;
	register_write(0x30, regData);

	regData = 0xE0;
	register_write(0xD8, regData);

	delayforworks(5*1000);

	regData = 0x00;
	register_write(0xD8, regData);

	regData = 0x00;
	register_write(0x30, regData);
}

void SH5001::reset()
{
	soft_reset();
	drive_start();	
	adc_reset();
	cva_reset();

	delayforworks(200*1000);
    acc_reset();
}

void SH5001::osc_freq()
{
	unsigned char regData = 0;

	regData = register_read(0xDA);
	if((regData & 0x07) != 0x07)
	{
		regData |= 0x07;
		regData = register_read(0xDA);
	}

}

int SH5001::init()
{
    ops = static_cast<struct i2c_master_s*>(dbind(ops_name));
    ops_valid = (ops != nullptr);

    uint8_t id, i;
    if (ops_valid) {
        while((id != 0xA1) && (i++ < 3))
        {
            id = register_read(SH5001_CHIP_ID);
            if((i == 3) && (id != 0xA1)) {
                ops_valid = false;
                return -1;
            }
        }
    }

    if (!ops_valid) return false;

    reset();
    osc_freq();
    return config();
}

void SH5001::config_imu_data_stable()
{
	unsigned char regData = 0;
	regData = register_read(SH5001_ACC_CONF0);
	regData &= 0xFE;
	register_write(SH5001_ACC_CONF0, regData);
	regData |= 0x1;
	register_write(SH5001_ACC_CONF0, regData);

    regData = register_read(SH5001_GYRO_CONF0);
    regData &= 0xFE;
    register_write(SH5001_GYRO_CONF0, regData);
    regData |= 0x1;
    register_write(SH5001_GYRO_CONF0, regData);
}

bool SH5001::config()
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

    config_imu_data_stable();
    return success;
}

void SH5001::get_accel()
{
	unsigned char reg_data[6]={0};

	register_reads(SH5001_ACC_XL, &reg_data[0], 6);

	accel_origin[0] = ((short)reg_data[1] << 8) | reg_data[0];
	accel_origin[1] = ((short)reg_data[3] << 8) | reg_data[2];
	accel_origin[2] = ((short)reg_data[5] << 8) | reg_data[4];
}

void SH5001::get_gyro()
{
	unsigned char reg_data[6]={0};

	register_reads(SH5001_GYRO_XL, &reg_data[0], 6);

	gyro_origin[0] = ((short)reg_data[1] << 8) | reg_data[0];
	gyro_origin[1] = ((short)reg_data[3] << 8) | reg_data[2];
	gyro_origin[2] = ((short)reg_data[5] << 8) | reg_data[4];
}

void SH5001::get_imu()
{
	unsigned char reg_data[12]={0};

	register_reads(SH5001_ACC_XL, &reg_data[0], 12);

	accel_origin[0] = ((short)reg_data[1] << 8) | reg_data[0];
	accel_origin[1] = ((short)reg_data[3] << 8) | reg_data[2];
	accel_origin[2] = ((short)reg_data[5] << 8) | reg_data[4];
	gyro_origin[0] = ((short)reg_data[7] << 8) | reg_data[6];
	gyro_origin[1] = ((short)reg_data[9] << 8) | reg_data[8];
	gyro_origin[2] = ((short)reg_data[11] << 8) | reg_data[10];

    // printf("[%d, %d, %d, %d, %d, %d]\r\n", 
    //     accel_origin[0], accel_origin[1], accel_origin[2],
    //     gyro_origin[0], gyro_origin[1], gyro_origin[2]);
}

void SH5001::run()
{
    get_imu();

    for (int i = 0; i < 3; i++) {
        gyro_rad[i] = gyro_origin[i] * Gyro_2000An_SCALE * (3.1415926 / 180.0f);
        accel_m2_s[i] =  accel_origin[i] * ACC_16G_SCALE * 9.79134f;
    }
}