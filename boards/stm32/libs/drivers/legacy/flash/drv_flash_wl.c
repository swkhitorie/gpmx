#include "drv_flash.h"

#define FLASH_PAGE_SIZE              (2*1024)
#define STM32_FLASH_START_ADRESS     (0x08000000)
#define STM32_FLASH_END_ADDRESS      (0x0803ffff)

#define FH_ALIGN_DOWN(size, align)      ((size) & ~((align) - 1))

static uint32_t GetPage(uint32_t addr)
{
    uint32_t page = 0;
    page = FH_ALIGN_DOWN(addr-STM32_FLASH_START_ADRESS, FLASH_PAGE_SIZE)/FLASH_PAGE_SIZE;
    return page;
}

/**
 * Read data from flash.
 * @note This operation's units is word.
 */
int stm32_flash_read(uint32_t addr, uint8_t *buf, size_t size)
{
    size_t i;

    if ((addr + size) > STM32_FLASH_END_ADDRESS) {
        return -1;
    }

    for (i = 0; i < size; i++, buf++, addr++) {
        *buf = *(uint8_t *) addr;
    }

    return size;
}

/**
 * Write data to flash.
 * @note This operation's units is word.
 * @note This operation must after erase. @see flash_erase.
 */

int stm32_flash_write(uint32_t addr, const uint8_t *buf, size_t size)
{
    size_t i, j;
    int result = 0;
    uint64_t write_data = 0, temp_data = 0;

    if ((addr + size) > STM32_FLASH_END_ADDRESS) {
        return -1;
    }

    if(addr % 8 != 0) {
        return -2;
    }

    HAL_FLASH_Unlock();

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR);

    if (size < 1) {
        return -3;
    }

    for (i = 0; i < size;) {

        if ((size - i) < 8) {
            for (j = 0; (size - i) > 0; i++, j++) {
                temp_data = *buf;
                write_data = (write_data) | (temp_data << 8 * j);
                buf ++;
            }
        } else {
            for (j = 0; j < 8; j++, i++) {
                temp_data = *buf;
                write_data = (write_data) | (temp_data << 8 * j);
                buf ++;
            }
        }

        /* write data */
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, write_data) == HAL_OK) {
            /* Check the written value */
            if (*(uint64_t*)addr != write_data) {
                result = -4;
                goto __exit;
            }
        } else {
            result = -5;
            goto __exit;
        }

        temp_data = 0;
        write_data = 0;

        addr += 8;
    }

__exit:
    HAL_FLASH_Lock();
    if (result != 0) {
        return result;
    }

    return size;
}

/**
 * Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 */
int stm32_flash_erase(uint32_t addr, size_t size)
{
    int result = 0;
    uint32_t PAGEError = 0;

    /*Variable used for Erase procedure*/
    FLASH_EraseInitTypeDef EraseInitStruct;

    if ((addr + size) > STM32_FLASH_END_ADDRESS) {
        return -1;
    }

    HAL_FLASH_Unlock();

    /* Fill EraseInit structure*/
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Page        = GetPage(addr);
    EraseInitStruct.NbPages     = (size + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
        result = -2;
        goto __exit;
    }

__exit:
    HAL_FLASH_Lock();

    if (result != 0) {
        return result;
    }

    return size;
}

