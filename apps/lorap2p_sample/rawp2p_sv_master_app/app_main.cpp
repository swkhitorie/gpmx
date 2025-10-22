#include "app_main.h"

#include "lorap2p.h"


struct __nvm_data _g_nvm;

int main(int argc, char *argv[])
{
    board_init();
    board_bsp_init();

    nvm_read(&_g_nvm);
    if (APP_LOAD_ADDRESS == APP_A_LOAD_FLASH_ADDR) {
        printf("appa version:%08x crc:%08x sz:%08x\r\n", 
            _g_nvm.app_a.version, _g_nvm.app_a.crc, _g_nvm.app_a.size);
    } else if (APP_LOAD_ADDRESS == APP_B_LOAD_FLASH_ADDR) {
        printf("appb version:%08x crc:%08x sz:%08x\r\n", 
            _g_nvm.app_b.version, _g_nvm.app_b.crc, _g_nvm.app_b.size);
    }

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

