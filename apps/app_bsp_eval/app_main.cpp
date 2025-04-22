#include <board_config.h>
#include <stdio.h>
#include <stdint.h>

#include <drivers/imu/sh5001/sh5001.hpp>

SH5001 imu("/sensor_i2c");

int main(int argc, char *argv[])
{
    board_init();
    hrt_init();

    int ret = imu.init();
    if (ret) {
        printf("[sh5001] init success\r\n");
    } else {
        printf("[sh5001] init failed\r\n");
    }

    uint32_t m = HAL_GetTick();
    for (;;) {
        if (HAL_GetTick() - m >= 10) {
            m = HAL_GetTick();

            imu.run();

            printf("%.6f| %.4f, %.4f, %.4f, %.4f, %.4f, %.4f\r\n",
                hrt_absolute_time()/1e6f,
                imu.accel_m2_s[0], imu.accel_m2_s[1], imu.accel_m2_s[2],
                imu.gyro_rad[0], imu.gyro_rad[1], imu.gyro_rad[2]);

            // board_debug();
        }
    }
}

