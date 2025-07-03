#include <board_config.h>
#include <stdio.h>
#include <stdint.h>

#include <IST8310.hpp>
#include <icm42688_test.h>
#include "px_upload/px_upload.h"

IST8310 mag("/sensor_i2c");
int16_t data[3];

upload_msg_t rst_msg;
uint8_t ch;

int main(int argc, char *argv[])
{
    board_init();
    board_bsp_init();

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

