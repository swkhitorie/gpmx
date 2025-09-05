#include "nvm_board.h"
#include <drv_flash.h>

static uint32_t
crc32(const uint8_t *src, unsigned len, unsigned state)
{
	static uint32_t crctab[256];

	/* check whether we have generated the CRC table yet */
	/* this is much smaller than a static table */
	if (crctab[1] == 0) {
		for (unsigned i = 0; i < 256; i++) {
			uint32_t c = i;

			for (unsigned j = 0; j < 8; j++) {
				if (c & 1) {
					c = 0xedb88320U ^ (c >> 1);

				} else {
					c = c >> 1;
				}
			}

			crctab[i] = c;
		}
	}

	for (unsigned i = 0; i < len; i++) {
		state = crctab[(state ^ src[i]) & 0xff] ^ (state >> 8);
	}

	return state;
}

uint32_t fm_load_crc_32(uint32_t addr, uint32_t size)
{
    // compute CRC of the programmed area
    uint32_t sum = 0;
    uint32_t bytes = 0;
    uint32_t bytes_read = 0;

    for (unsigned p = 0; p < size; p += 4) {

        bytes_read = stm32_flash_read(p+addr, (uint8_t *)&bytes, 4);
        if (bytes_read != 4) {
            // printf("[flash] read error %x \r\n", bytes_read);
        }

        sum = crc32((uint8_t *)&bytes, sizeof(bytes), sum);
    }

    return sum;
}

void nvm_write(struct __nvm_data *pnvm)
{
    int ret = 0;

    ret = stm32_flash_erase(NVM_FLASH_ADDR, sizeof(struct __nvm_data));
    if (ret != sizeof(struct __nvm_data)) {
        // printf("[bl] nvm erase error \r\n");
    }

    ret = stm32_flash_write(NVM_FLASH_ADDR, (uint8_t *)pnvm, sizeof(struct __nvm_data));
    if (ret != sizeof(struct __nvm_data)) {
        // printf("[bl] nvm write error \r\n");
    }
}

void nvm_read(struct __nvm_data *pnvm)
{
    int rsz = stm32_flash_read(NVM_FLASH_ADDR, (uint8_t *)pnvm, sizeof(struct __nvm_data));
    if (rsz != sizeof(struct __nvm_data)) {
        // printf("[bl] nvm read error \r\n");
    }
}
