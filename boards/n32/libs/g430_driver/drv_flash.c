#include <drv_flash.h>

#define FLASH_PAGE_SIZE              (2*1024)
#define N32_FLASH_START_ADRESS       (0x08000000)

#define FH_ALIGN_DOWN(size, align)      ((size) & ~((align) - 1))

static uint32_t get_page(uint32_t addr)
{
    uint32_t page = 0;

    page = FH_ALIGN_DOWN(addr-N32_FLASH_START_ADRESS, FLASH_PAGE_SIZE)/FLASH_PAGE_SIZE;

    return page;
}

int n32_flash_erase(uint32_t addr, size_t size)
{
    int result = 0;
    uint32_t end_addr = addr + size;

    FLASH_Unlock();

    while (addr < end_addr) {

        if (FLASH_One_Page_Erase(addr) != FLASH_EOP) {
            result = -1;
            goto __exit;
        }

        addr += FLASH_PAGE_SIZE;
    }

    FLASH_Lock();

__exit:
    if (result != 0) {
        return result;
    }

    return size;
}


int n32_flash_read(uint32_t addr, uint8_t *buf, size_t size)
{
    size_t i;

    for (i = 0; i < size; i++, buf++, addr++) {
        *buf = *(uint8_t *) addr;
    }

    return size;
}

int n32_flash_write(uint32_t addr, const uint8_t *buf, size_t size)
{
    int result          = 0;
    uint32_t end_addr   = addr + size;

    if (addr % 4 != 0) {
        // LOG_E("write addr must be 4-byte alignment");
        return -1;
    }

    FLASH_Unlock();

    while (addr < end_addr) {

        if (FLASH_Word_Program(addr, *((uint32_t *)buf)) == FLASH_EOP) {

            if (*(uint32_t *)addr != *(uint32_t *)buf) {
                result = -2;
                break;
            }
            addr += 4;
            buf  += 4;
        } else {
            result = -2;
            break;
        }
    }

    FLASH_Lock();

    if (result != 0)
    {
        return result;
    }

    return size;
}
