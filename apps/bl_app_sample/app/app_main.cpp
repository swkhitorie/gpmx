#include <board_config.h>
#include <stdio.h>
#include <stdint.h>

#include <device/serial.h>

struct uart_dev_s *bus;
uint8_t rst_buf[64];
bool rst_cmd_match(const uint8_t *buf)
{
    if (buf[0] == 0x55 && buf[1] == 0x05 && buf[2] == 0x00 && buf[3] == 0x01 &&
        buf[4] == 0x52 && buf[5] == 0x45 && buf[6] == 0x53 && buf[7] == 0x45 &&
        buf[8] == 0x54 && buf[9] == 0x89 && buf[10] == 0x0A)
    {
        return true;
    }

    return false;
}

int main(int argc, char *argv[])
{
    int sz = 0;

    board_init();
    board_bsp_init();

    bus = serial_bus_get(1);

    uint32_t m = HAL_GetTick();
    for (;;) {

        sz = SERIAL_RDBUF(bus, rst_buf, 64);
        if (sz == 11) {
            if (rst_cmd_match(rst_buf)) {
                board_reboot();
            }
        }

        if (HAL_GetTick() - m >= 100) {
            m = HAL_GetTick();

            printf("[app] running test with rst cmd\r\n");

            board_debug();
        }
    }
}

