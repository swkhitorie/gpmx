#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <board_config.h>
#include <drv_flash.h>

#include "nvm_board.h"

#define __BOOT_VERSION_MAIN   (0x01U) /*!< [31:24] main version */
#define __BOOT_VERSION_SUB1   (0x02U) /*!< [23:16] sub1 version */
#define BOOT_VERSION          ((__BOOT_VERSION_MAIN << 24) | (__BOOT_VERSION_SUB1 << 16))


static uint32_t app_load_select(struct __nvm_data *pnvm);
static void     app_load_jump(uint32_t load_addr);
static struct __nvm_data _g_nvm;
int main(int argc, char *argv[])
{
    board_init();
    board_bsp_init();

    nvm_read(&_g_nvm);

    printf("a: flag:%08x ver:%08x crc:%08x sz:%08x \r\n",
        _g_nvm.app_a.load_flag, _g_nvm.app_a.version, _g_nvm.app_a.crc, _g_nvm.app_a.size);

    printf("b: flag:%08x ver:%08x crc:%08x sz:%08x \r\n",
        _g_nvm.app_b.load_flag, _g_nvm.app_b.version, _g_nvm.app_b.crc, _g_nvm.app_b.size);

    if (_g_nvm.boot_version != BOOT_VERSION) {
        // write boot version
        _g_nvm.boot_version = BOOT_VERSION;

        nvm_write(&_g_nvm);
    }

    uint32_t app_load = app_load_select(&_g_nvm);

    app_load_jump(app_load);

    uint32_t m = HAL_GetTick();
    for (;;) {

        if (board_elapsed_tick(m) > 100) {
            m = HAL_GetTick();
            board_led_toggle(0);

            printf("boot ver: %08x\r\n", _g_nvm.boot_version);
        }
    }

}


uint32_t app_load_select(struct __nvm_data *pnvm)
{
    uint32_t load_addr;

    if (pnvm->app_a.load_flag == 0xffffffff &&
        pnvm->app_a.version == 0xffffffff &&
        pnvm->app_a.crc == 0xffffffff &&
        pnvm->app_a.size == 0xffffffff) {
        return APP_B_LOAD_FLASH_ADDR;
    }

    if (pnvm->app_b.load_flag == 0xffffffff &&
        pnvm->app_b.version == 0xffffffff &&
        pnvm->app_b.crc == 0xffffffff &&
        pnvm->app_b.size == 0xffffffff) {
        return APP_A_LOAD_FLASH_ADDR;
    }

    uint32_t app_a_crc_cal = fm_load_crc_32(APP_A_LOAD_FLASH_ADDR, pnvm->app_a.size);
    uint32_t app_a_load_flag = pnvm->app_a.load_flag;
    uint32_t app_a_version = pnvm->app_a.version;

    uint32_t app_b_crc_cal = fm_load_crc_32(APP_B_LOAD_FLASH_ADDR, pnvm->app_b.size);
    uint32_t app_b_load_flag = pnvm->app_b.load_flag;
    uint32_t app_b_version = pnvm->app_b.version;

    uint8_t ret_a = ((app_a_crc_cal == pnvm->app_a.crc) && (0x00000000 == app_a_load_flag));
    uint8_t ret_b = ((app_b_crc_cal == pnvm->app_b.crc) && (0x00000000 == app_b_load_flag));
    uint8_t a_version_high = (app_a_version > app_b_version);
    uint8_t ab_version_equal = (app_a_version == app_b_version);

    if (ret_a == 1 && ret_b == 0) {
        load_addr = APP_A_LOAD_FLASH_ADDR; //load a
    } else if (ret_a == 0 && ret_b == 1) {
        load_addr = APP_B_LOAD_FLASH_ADDR; //load b
    } else if (ret_a == 1 && ret_b == 1 && a_version_high == 1 && ab_version_equal == 0) {
        load_addr = APP_A_LOAD_FLASH_ADDR; //load a
    } else if (ret_a == 1 && ret_b == 1 && a_version_high == 0 && ab_version_equal == 0) {
        load_addr = APP_B_LOAD_FLASH_ADDR; //load b
    } else if (ret_a == 1 && ret_b == 1 && a_version_high == 0 && ab_version_equal == 1) {
        load_addr = APP_A_LOAD_FLASH_ADDR; //load a
    } else {
        printf("wtf app map \r\n");
    }

    return load_addr;
}

static void
do_jump(uint32_t stacktop, uint32_t entrypoint)
{
	asm volatile(
		"msr msp, %0  \n"
		"bx %1  \n"
		: : "r"(stacktop), "r"(entrypoint) :);

	// just to keep noreturn happy
	for (;;) ;
}

void app_load_jump(uint32_t load_addr)
{
    const uint32_t *app_base = (const uint32_t *)load_addr;

    board_bsp_deinit();

    board_deinit();

    __set_CONTROL(0);

    /* switch exception handlers to the application */
    SCB->VTOR = load_addr;

    /* extract the stack and entrypoint from the app vector table and go */
    do_jump(app_base[0], app_base[1]);

    while (1) {}
}

