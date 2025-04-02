#include <board_config.h>
#include <stdio.h>
#include <stdint.h>

#include <drivers/imu/mpu6050/mpu6050.hpp>
#include <drivers/magnetometers/hmc5883/hmc5883l.hpp>
#include <drivers/drv_hrt.h>

#define APP_BSP_EVAL_EXAMPLE  0

#if (APP_BSP_EVAL_EXAMPLE == 2)
#include "ist8310_test.h"
#endif

#if (APP_BSP_EVAL_EXAMPLE == 3)
#include "icm42688_test.h"
#endif

#if (APP_BSP_EVAL_EXAMPLE == 5)
#include "l3gd20_test.h"
#endif

#if (APP_BSP_EVAL_EXAMPLE == 6)
#include "mpu6050_test.h"
#endif

MPU6050 imua("/sensor_i2c");
HMC5883 compassa("/sensor_i2c");
int main(int argc, char *argv[])
{
    board_init();
    hrt_init();

#if (APP_BSP_EVAL_EXAMPLE == 2)
    int16_t mag_data[3];
    if (ist8310_init()) {
        printf("ist8310 init success \r\n");
    } else {
        printf("ist8310 init failed \r\n");
    }
#endif

#if (APP_BSP_EVAL_EXAMPLE == 3)
    int16_t accel_data[3];
    icm42688_init();
#endif
    uint8_t buff_debug[256];

#if (APP_BSP_EVAL_EXAMPLE == 5)
    l3gd20_init();
#endif

#if (APP_BSP_EVAL_EXAMPLE == 6)
    int16_t accel[3];
    if (mpu6050_init()) {
        printf("mpu6050 init success \r\n");
    } else {
        printf("mpu6050 init failed \r\n");
    }
#endif

    int ret = imua.init();
    printf("imu init ret: %d \r\n", ret);

    ret = compassa.init();
    printf("compass init ret: %d \r\n", ret);

    uint32_t m = HAL_GetTick();
    for (;;) {
        if (HAL_GetTick() - m >= 100) {
            m = HAL_GetTick();

            imua.run();
            printf("%.3f, %.3f, %.3f, %.3f, %.3f, %.3f\r\n", imua.accel_g[0], imua.accel_g[1], imua.accel_g[2]
            , imua.gyro_deg[0], imua.gyro_deg[1], imua.gyro_deg[2]);

            compassa.run();
            printf("%.3f, %.3f, %.3f\r\n", compassa.mag_gaus[0], compassa.mag_gaus[1], compassa.mag_gaus[2]);
        #if (APP_BSP_EVAL_EXAMPLE == 2)
            ist8310_mag(mag_data);
            printf("[ist8310] %d %d %d \r\n", mag_data[0], mag_data[1], mag_data[2]);
        #endif

        #if (APP_BSP_EVAL_EXAMPLE == 3)
            icm42688_read(&accel_data[0]);
            printf("[icm42688] %d %d %d \r\n", accel_data[0], accel_data[1], accel_data[2]);
        #endif

        #if (APP_BSP_EVAL_EXAMPLE == 4)
            int size = fread(buff_debug, 1, 256, stdin);
            if (size > 0) {
                for (int i = 0; i < size; i++) {
                    printf("%c", buff_debug[i]);
                }
                printf("\r\n");
            }
        #endif

        #if (APP_BSP_EVAL_EXAMPLE == 5)
            l3gd20_init();
        #endif

        #if (APP_BSP_EVAL_EXAMPLE == 6)
            // mpu6050_accel(&accel[0]);
            // printf("mpu accel: %d, %d, %d \r\n", accel[0], accel[1], accel[2]);
        #endif

            board_debug();
        }
    }
}

