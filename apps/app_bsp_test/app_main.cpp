#include <board_config.h>
#include <stdio.h>
#include <stdint.h>
#include <drivers/drv_hrt.h>

int main(int argc, char *argv[])
{
    board_init();
    hrt_init();

    uint32_t m = HAL_GetTick();
    for (;;) {
        if (HAL_GetTick() - m >= 10) {
            m = HAL_GetTick();
            board_debug();
        }
    }
}

