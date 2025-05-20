#include <board_config.h>
#include <stdio.h>
#include <stdint.h>

#include <drivers/magnetometers/ist8310/IST8310.hpp>
#include <drivers/imu/icm42688p/icm42688_test.h>

IST8310 mag("/sensor_i2c");
int16_t data[3];

int main(int argc, char *argv[])
{
    board_init();
    hrt_init();

    mag.init();

    icm42688_init();

    uint32_t m = HAL_GetTick();
    for (;;) {
        if (HAL_GetTick() - m >= 100) {
            m = HAL_GetTick();

            mag.run();
            mag.print_status();

            icm42688_read(&data[0]);
            printf("[icm42688p] %d, %d, %d \r\n", data[0], data[1], data[2]);

            board_debug();
        }
    }
}

