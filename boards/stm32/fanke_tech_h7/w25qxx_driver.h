
#ifndef W25QXX_QUADSPI_DRIVER_H_
#define W25QXX_QUADSPI_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>

#include <lfs.h>
#include <board_config.h>
#define W25QXX_WAIT_TIMESTAMP          HAL_GetTick

#define W25QXX_PAGESIZE                256
#define W25QXX_SECTORSIZE              4096

#define W25QXX_CHIPERASE_TIMEOUT_MAX   100000U
#define W25QXX_MEM_ADDR                0x90000000

#define W25Q64_FLASHSIZE               0x800000  // 64M bits -> 8M bytes
#define W25Q64_FLASHID                 0xEF4017

enum w25qxx_flag {
    QSPI_W25QXX_OK = 0x00,
    QSPI_W25QXX_ERROR_INIT = 0x01,
    QSPI_W25QXX_ERROR_WRITE_ENABLE,
    QSPI_W25QXX_ERROR_AUTOPOLLING,
    QSPI_W25QXX_ERROR_ERASE,
    QSPI_W25QXX_ERROR_TRANSMIT,
    QSPI_W25QXX_MEMORY_MAPPED
};

enum w25qxx_cmd {
    W25QXX_CMD_ENABLE_RESET = 0x66,
    W25QXX_CMD_RESET_DEVICE = 0x99,
    W25QXX_CMD_JEDECID = 0x9F,
    W25QXX_CMD_WRITE_ENABLE = 0x06,
    W25QXX_CMD_SECTOR_ERASE = 0x20,
    W25QXX_CMD_BLOCKERASE_32K = 0x52,
    W25QXX_CMD_BLOCKERASE_64K = 0xD8,
    W25QXX_CMD_CHIPERASE = 0xC7,
    W25QXX_CMD_QUADINPUT_PAGEPROGRAM = 0x32,
    W25QXX_CMD_FASTREAD_QUADIO = 0xEB,
    W25QXX_CMD_READSTATUS_REG1 = 0x05,
};

enum w25qxx_status_reg_bits {
    W25QXX_STATUS_REG1_BUSY = 0x01,
    W25QXX_STATUS_REG1_WEL = 0x02,
};

#ifdef __cplusplus
extern "C" {
#endif

bool      w25qxx_init(int nbus);
bool      w25qxx_reset();
uint32_t  w25qxx_readid();

uint8_t   w25qxx_sector_erase(uint32_t addr); // refer option times: 45ms
uint8_t   w25qxx_blockerase_32k(uint32_t addr); // refer option times: 120ms
uint8_t   w25qxx_blockerase_64k(uint32_t addr); // refer option times: 150ms
uint8_t   w25qxx_chiperase(); // refer option times: 20s

uint8_t   w25qxx_writepage(uint32_t addr, uint8_t *p, uint16_t size);	// page write, max 256bytes
uint8_t   w25qxx_writebuffer(uint32_t addr, uint8_t *p, uint32_t size);
uint8_t   w25qxx_readbuffer(uint32_t addr, uint8_t *p, uint32_t size);
void      w25qxx_debug(int nbus);

struct lfs_config *w25qxx_get_lfs_ops();

#ifdef __cplusplus
}
#endif

#endif
