#include <board_config.h>
#include <stdio.h>
#include <stdint.h>

// #include <drivers/drv_hrt.h>
// #include <drivers/barometer/ms5611/ms5611.hpp>
// #include <drivers/magnetometers/ist8310/IST8310.hpp>


int main(int argc, char *argv[])
{
    board_init();

    uint32_t m = HAL_GetTick();
    for (;;) {
        if (HAL_GetTick() - m >= 100) {
            m = HAL_GetTick();


            board_debug();
        }
    }
}

