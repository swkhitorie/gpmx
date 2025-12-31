#include <board_config.h>
#include <stdio.h>
#include <stdint.h>
#include <board_usb_cdc.h>

void reboot_detect()
{
    uint8_t cmd_rx[64];
    int sz = board_cdc_acm_read(0, cmd_rx, 8);
    if (sz >= 8) {
        if (cmd_rx[0] == 'r' && cmd_rx[1] == 'e' &&
            cmd_rx[2] == 'b' && cmd_rx[3] == 'o' &&
            cmd_rx[4] == 'o' && cmd_rx[5] == 't' &&
            cmd_rx[6] == '\r' && cmd_rx[7] == '\n') {
            board_reboot();
        }
    }
}

void fault_test_by_div0()
{
    volatile int *SCB_CCR = (volatile int *)0xE000ED14; // SCB->CCR
    int x, y, z;

    *SCB_CCR |= (1 << 4); /* bit4: DIV_0_TRP */ 

    x = 10;
    y = 0;
    z = x / y;

    board_printf("z:%d\r\n",z);
}

int main(int argc, char *argv[])
{
    board_init();
    board_bsp_init();

    // fault_test_by_div0();

    uint32_t m = HAL_GetTick();
    for (;;) {
        if (HAL_GetTick() - m >= 100) {
            m = HAL_GetTick();

            reboot_detect();
            board_debug();

            int length = 3;

            board_printf("std print write: %d\r\n", length);
            board_stream_printf(1, "board print write: %d\r\n", length);
        }
    }
}

