#ifndef _NVM_BOARD_H_
#define _NVM_BOARD_H_

#include <stdint.h>

#define NVM_FLASH_ADDR              (0x0800b800)
#define APP_A_LOAD_FLASH_ADDR       (0x0800c000)
#define APP_B_LOAD_FLASH_ADDR       (0x08026000)

struct __load_app {
    uint32_t load_flag; // cannot load(default): 0xff, load-able: 0x00
    uint32_t version;
    uint32_t crc;
    uint32_t size;
};

struct __nvm_data {
    uint32_t boot_version;
    uint32_t __pad[1];

    struct __load_app app_a;
    struct __load_app app_b;
};

#ifdef __cplusplus
extern "C" {
#endif

uint32_t fm_load_crc_32(uint32_t addr, uint32_t size);
void nvm_write(struct __nvm_data *pnvm);
void nvm_read(struct __nvm_data *pnvm);

#ifdef __cplusplus
}
#endif

#endif
