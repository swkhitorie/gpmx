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

int main(int argc, char *argv[])
{
    board_init();
    board_bsp_init();

    uint32_t m = HAL_GetTick();
    for (;;) {
        if (HAL_GetTick() - m >= 100) {
            m = HAL_GetTick();

            reboot_detect();
            board_debug();
            printf("hello2\r\n");
        }
    }
}

