#ifndef DRV_QSPI_H_
#define DRV_QSPI_H_

#if defined (DRV_BSP_H7)
#include <stdint.h>
#include "drv_common.h"
#include "drv_gpio.h"
#include "drv_pin_h7.h"

// #define QSPI_W25QXX_ENABLE_RW_TEST

struct drv_qspi_msg {
    const void *send_buf;
    void *recv_buf;
    size_t length;
    struct {
        uint8_t content;
        uint8_t qspi_lines;
    } instruction;
    struct {
        uint32_t content;
        uint8_t size;
        uint8_t qspi_lines;
    } address;
    struct {
        uint32_t content;
        uint8_t size;
        uint8_t qspi_lines;
    } alternate_bytes;
    uint32_t dummy_cycles;
    uint8_t qspi_data_lines;
};

void drv_qspi_init(uint8_t flashNum, uint8_t prescaler, uint32_t flashsize,
                uint8_t clks, uint8_t io0s, uint8_t io1s, uint8_t io2s,
                uint8_t io3s, uint8_t ncss);

/****************************************************************************
 * W25Qxx QSPI Flash Driver
 ****************************************************************************/
#define W25QXX_PAGESIZE                256
#define W25QXX_FLASHSIZE               0x800000  // size of W25Q64, 8M bytes
#define W25QXX_FLASHID                 0xEF4017
#define W25QXX_CHIPERASE_TIMEOUT_MAX   100000U
#define W25QXX_MEM_ADDR                0x90000000

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

int8_t    w25qxx_reset();
uint32_t  w25qxx_readid();
bool      w25qxx_init();
uint8_t   w25qxx_memory_mapped_mode();

uint8_t   w25qxx_sector_erase(uint32_t addr); // refer option times: 45ms
uint8_t   w25qxx_blockerase_32k(uint32_t addr); // refer option times: 120ms
uint8_t   w25qxx_blockerase_64k(uint32_t addr); // refer option times: 150ms
uint8_t   w25qxx_chiperase(); // refer option times: 20s

uint8_t   w25qxx_writepage(uint8_t *p, uint32_t addr, uint16_t size);	// page write, max 256bytes
uint8_t   w25qxx_writebuffer(uint8_t *p, uint32_t addr, uint32_t size);
uint8_t   w25qxx_readbuffer(uint8_t *p, uint32_t addr, uint32_t size);

#ifdef QSPI_W25QXX_ENABLE_RW_TEST
void     w25qxx_rwtest(uint32_t testaddr, char *debug);
#endif

#endif  // end with DRV_BSP_H7

#endif  // end with DRV_QSPI_H_
