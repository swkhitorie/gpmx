#ifndef FM25VXX_SPIFLASH_H_
#define FM25VXX_SPIFLASH_H_

#include <stdint.h>

#define FM25V01_MAX_SIZE       (16*1024)
#define FM25V01_ADDR_MAX       (0x3fff)

#define FM25V02_MAX_SIZE       (32*1024)
#define FM25V02_ADDR_MAX       (0x7fff)

enum FM25V01_CMD {
    FM25_WREN = 0x06,       // Set Write enable latch
    FM25_WRDI = 0x04,       // Reset Write enable latch
    FM25_RDSR = 0x05,       // Read Status Register
    FM25_WRSR = 0x01,       // Write Status Register
    FM25_READ = 0x03,       // Read Memory data
    FM25_FSTRD = 0x0B,      // Fast Read memory data
    FM25_WRITE = 0x02,      // Write memory data
    FM25_SLEEP = 0xB9,      // Enter sleep mode
    FM25_RDID = 0x9F,       // Read device ID
};

enum FM25V01_STATUS_REG {
    FM25_STATUS_WEL = 0x02,  // '1' -> Write Enabled
    FM25_STATUS_WPEN = 0x80, //  enable the write protect pin
};

#ifdef __cplusplus
extern "C" {
#endif

int fm25_flash_init(int spibus);
int fm25_flash_write(uint16_t addr, const uint8_t *p, uint32_t sz);
int fm25_flash_read(uint16_t addr, uint8_t *p, uint32_t sz);
int fm25_flash_fread(uint16_t addr, uint8_t *p, uint32_t sz);

int fm25_flash_test(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif

