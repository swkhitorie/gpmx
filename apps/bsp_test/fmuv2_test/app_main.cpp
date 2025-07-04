#include <board_config.h>
#include <stdio.h>
#include <stdint.h>

#include "l3gd20_test.h"

int main(int argc, char *argv[])
{
    board_init();
    board_bsp_init();

    uint32_t m = HAL_GetTick();
    for (;;) {
        if (HAL_GetTick() - m >= 100) {
            m = HAL_GetTick();

            l3gd20_init();

            printf("hello v2\r\n");
            board_debug();
        }
    }
}

