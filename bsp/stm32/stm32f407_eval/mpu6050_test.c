#include "mpu6050_test.h"

void wait_block()
{
    HAL_Delay(2);
}

uint8_t aaid;
uint8_t checkid()
{
    aaid = 0;
	read_register(MPU_DEVICE_ID_REG, &aaid, 1, 0);
    return aaid;
}

bool mpu6050_init()
{
    int idx;
    uint16_t reg_write_list[6] = {
        MPU_USER_CTRL_REG, MPU_PWR_MGMT2_REG,
        MPU_SAMPLE_RATE_REG, MPU_CFG_REG,
        MPU_ACCEL_CFG_REG, MPU_GYRO_CFG_REG,
    };
    uint8_t reg_write_val[6] = {
        0x00, 0x00, 
        0x00, 0x07,
        0x18, 0x18,
    };
    uint8_t reg_write_check_val[6] = {0x00};

    write_register(MPU_PWR_MGMT1_REG, 0x80); //0x80
    wait_block();
    write_register(MPU_PWR_MGMT1_REG, 0x01); //0x80
    wait_block();
    write_register(MPU_SIGPATH_RST_REG, 0x07); //0x80
    wait_block();

    for (idx = 0; idx < 6; idx++) {
        write_register(reg_write_list[idx], reg_write_val[idx]);
        wait_block();
    }

    for (idx = 0; idx < 6; idx++) {
        read_register(reg_write_list[idx], &reg_write_check_val[idx], 1, 0);
        if (reg_write_check_val[idx] != reg_write_val[idx]) {
            break;
        }
        wait_block();
    }

    if (idx != 6) {
        printf("[mpu60xx] check false %d \r\n", idx);
        return false;
    }
	uint8_t tmpid = checkid();
    return (tmpid == MPU_ID_VALUE_1 || 
        tmpid == MPU_ID_VALUE_2 || 
        tmpid == MPU_ID_VALUE_3 || 
        tmpid == MPU_ID_VALUE_4);
}

uint8_t rawgyro[6];
uint8_t rawtemp[2];
uint8_t rawacc[6];
uint8_t rawall[14];

bool mpu6050_accel(int16_t *accel)
{
    read_register(MPU_ACCEL_XOUTH_REG, &rawacc[0], 6, 1);
    accel[0] = (rawacc[0] << 8) | rawacc[1];
    accel[1] = (rawacc[2] << 8) | rawacc[3];
    accel[2] = (rawacc[4] << 8) | rawacc[5];
}

bool mpu6050_gyro(int16_t *gyro)
{
    read_register(MPU_GYRO_XOUTH_REG, &rawgyro[0], 6, 1);
    gyro[0] = (rawgyro[0] << 8) | rawgyro[1];
    gyro[1] = (rawgyro[2] << 8) | rawgyro[3];
    gyro[2] = (rawgyro[4] << 8) | rawgyro[5];
}



