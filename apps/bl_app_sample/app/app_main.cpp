#include <board_config.h>
#include <stdio.h>
#include <stdint.h>

#include "ymdm_rcv.h"
#include <device/serial.h>

struct __ymodem_receiver yrv;
struct uart_dev_s *bus;
uint8_t bf_bl1[2048];

void ymodem_send(uint8_t c)
{
    SERIAL_SEND(bus, &c, 1);
}

int main(int argc, char *argv[])
{
    int sz = 0;

    board_init();
    board_bsp_init();

    bus = serial_bus_get(1);

    memset(&yrv, 0, sizeof(struct __ymodem_receiver));

    ymodem_cfg_snd_interface(&yrv, ymodem_send);
    yrv.state = YMODEM_SYNC_WAIT;

    uint32_t m = HAL_GetTick();
    for (;;) {

        if (HAL_GetTick() - m >= 100) {
            m = HAL_GetTick();

            printf("hello\r\n");


            sz = SERIAL_RDBUF(bus, bf_bl1, 1200);
            if (sz > 0 && sz != 11) {
                for (int i = 0; i < sz; i++) {
                    printf("%02X ", bf_bl1[i]);
                }
                printf("\r\n");
                ymodem_rx_process(&yrv, bf_bl1, sz);
            } else {
                if (yrv.state == YMODEM_SYNC_WAIT) {
                    ymodem_send(YMODEM_C);
                }
            }
            

            board_debug();
        }
    }
}

