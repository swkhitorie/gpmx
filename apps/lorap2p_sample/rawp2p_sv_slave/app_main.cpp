#include "app_main.h"

#include "lorap2p.h"

#include "ymdm_rcv.h"
#include <device/serial.h>
#include <drv_flash.h>
#include "nvm_board.h"

struct __ymodem_receiver   yrv; 
int                        total_ysize_cal = 0;
int                        total_ysize = 0;
struct uart_dev_s         *rbus;
uint8_t                    rbuf_rbuf[1400];
uint32_t                   crc_file;
uint32_t                   ybin_type;
uint32_t                   ybin_version;

volatile uint32_t   store_app_addr = 0x08020000;
volatile uint32_t   store_app_addr_wd = store_app_addr;

void get_version_from_ybin(const char *fname, uint32_t *version, uint32_t *apptype)
{
    uint32_t cversion = 0;
    uint32_t atype = 0;

    for (int i = strlen(fname); i >= 0; i--) {
        if (fname[i] == '.' && fname[i-1] == 'a') {
            atype = 0x01;
        }

        if (fname[i] == '.' && fname[i-1] == 'b') {
            atype = 0x02;
        }

        if ((fname[i] == 'a' ||  fname[i] == 'b') &&
            fname[i+1] == '.') {
            cversion |= (fname[i-1] - '0') << 16;
            cversion |= (fname[i-2] - '0') << 24;
        }
    }

    *version = cversion;
    *apptype = atype;
}

void ymodem_send(uint8_t c)
{
    SERIAL_SEND(rbus, &c, 1);
}

void ymodem_receiver_callback(uint32_t seq, uint8_t *p, uint16_t size)
{
    int ck_val = -1;

    if (seq == 1) {
        total_ysize = ymodem_filesz(&yrv);
        get_version_from_ybin(yrv.fname, &ybin_version, &ybin_type);
        if (ybin_type == 0x01) {
            printf("rcv app a ");
        } else if (ybin_type == 0x02) {
            printf("rcv app b ");
        }

        printf("total rcv filesz: %d, ready to flash\r\n", total_ysize);
        stm32_flash_erase(store_app_addr, total_ysize);
    }

    ck_val = stm32_flash_write(store_app_addr_wd, p, size);

    if (ck_val != size) {
        printf("error write flash %d %d \r\n", ck_val, size);
        while(1) {}
    }

    store_app_addr_wd += size;
    total_ysize_cal += size;

    printf("write rseq: %d, size: %d total: %d\r\n", seq, size, total_ysize_cal);
}

void firmware_rcv_process()
{
    int rsz = 0;
    rbus = serial_bus_get(1);

    printf("Ready to rcv firmware bin\r\n");

    ymodem_reset(&yrv);
    ymodem_cfg_snd_interface(&yrv, ymodem_send);
    ymodem_cfg_callback(&yrv, ymodem_receiver_callback);
    ymodem_start(&yrv);

	uint32_t m = HAL_GetTick();
    for (;;) {
        if (HAL_GetTick() - m >= 100) {
            m = HAL_GetTick();

            rsz = SERIAL_RDBUF(rbus, rbuf_rbuf, 1400);
            if (rsz > 0 && rsz != 11) {
                ymodem_rx_process(&yrv, rbuf_rbuf, rsz);
            } else {
                if (ymodem_state(&yrv) == YMODEM_SYNC_WAIT) {
                    ymodem_send(YMODEM_C);
                    HAL_Delay(100);
                }
            }
        }

        if (total_ysize_cal == total_ysize && ymodem_state(&yrv) == YMODEM_RCV_COMPLETED) {
            break;
        }
    }

    HAL_Delay(3000);
}



int main(int argc, char *argv[])
{
    board_init();
    board_bsp_init();

    firmware_rcv_process();

    crc_file = fm_load_crc_32(store_app_addr, total_ysize);

    get_version_from_ybin(yrv.fname, &ybin_version, &ybin_type);

    printf("crc:%08X, version:%08X, type:%d \r\n", crc_file, ybin_version, ybin_type);

    lorap2p_init();
    uint32_t m2 = HAL_GetTick();
    for (;;) {

        lorap2p_process();

        if (board_elapsed_tick(m2) > 100) {
            m2 = HAL_GetTick();
            board_led_toggle(0);
        }
    }

}

