#include "app_main.h"

#include "sx126x_test.h"

void gnss_pps_irq(void *p) 
{
}

int main(int argc, char *argv[])
{
    board_init();
    board_bsp_init();

    m_sx126x_init();

    uint32_t m1 = HAL_GetTick();

    for (;;) {


        if (HAL_GetTick() - m1 >= 500) {
            m1 = HAL_GetTick();
            board_led_toggle(1);
        }

    }
}

