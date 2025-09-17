#include <board_config.h>
#include <stdio.h>

#include <drv_flash.h>

uint32_t words_wr_test[8] = {
    0x11111111, 0x22222222, 0x33333333, 0x44444444,
    0x55555555, 0x66666666, 0x77777777, 0x88888888};
uint32_t words_rd_test[8];

int main(void)
{
    int ret = 0;
    board_init();
    board_bsp_init();

    int r1 = n32_flash_erase(0x0800f000, 2*1024);
    printf("erase flash: %d\r\n", r1);
    int r2 = n32_flash_write(0x0800f000, (const uint8_t *)&words_wr_test[0], 32);
    printf("write flash: %d\r\n", r2);
    int r3 = n32_flash_read(0x0800f000, (uint8_t *)&words_rd_test[0], 32);
    printf("read :\r\n");
    for (int i = 0; i < 8; i++) {
        printf("%08x ", words_rd_test[i]);
    }
    printf("\r\n");

    uint32_t m1 = board_get_tmstamp();
    while(1) {

        if (board_get_tmstamp() - m1 >= 1000) {
            m1 = board_get_tmstamp();
            board_led_toggle(2);
            printf("hello armv6 bin build in linux2\r\n");
        }
    }

    return 0;
}

