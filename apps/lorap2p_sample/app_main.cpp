#include "app_main.h"

#include "lorap2p.h"

int main(int argc, char *argv[])
{
    board_init();
    board_bsp_init();

    lorap2p_init();

    uint32_t m = HAL_GetTick();
    for (;;) {

        lorap2p_process();

        if (board_elapsed_tick(m) > 100) {
            m = HAL_GetTick();
            board_led_toggle(0);
        }
    }

}

