#include <board_config.h>
#include <stdio.h>
#include <stdint.h>

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

int main(int argc, char *argv[])
{
    board_init();

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

    uint32_t m = HAL_GetTick();
    for (;;) {
        if (HAL_GetTick() - m >= 1) {
            m = HAL_GetTick();

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
            mpu6050_accel(&accel[0]);
            printf("mpu accel: %d, %d, %d \r\n", accel[0], accel[1], accel[2]);
        #endif

            board_debug();
        }
    }
}

