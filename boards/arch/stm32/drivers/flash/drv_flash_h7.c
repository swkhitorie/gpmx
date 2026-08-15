

#include "drv_flash.h"

#define FLASH_PAGE_SIZE              (128*1024)
#define STM32_FLASH_START_ADRESS     (0x08000000)

int stm32_flash_read(uint32_t addr, uint8_t *buf, size_t size)
{
    size_t i;

    for (i = 0; i < size; i++, buf++, addr++)
    {
        *buf = *(uint8_t *) addr;
    }

    return size;
}

int stm32_flash_write(uint32_t addr, const uint8_t *buf, size_t size)
{
    int result = 0;
    uint32_t end_addr = addr + size - 1, write_addr;
    uint32_t write_granularity = FLASH_NB_32BITWORD_IN_FLASHWORD * 4;
    uint32_t write_size = write_granularity;
    uint8_t write_buffer[32] = {0};

    if (addr % 32 != 0) {
        return -1;
    }

    if (size < 1) {
        return -2;
    }

    HAL_FLASH_Unlock();
    write_addr = (uint32_t)buf;
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR);
    while (addr < end_addr) {

        if(end_addr - addr + 1 < write_granularity) {

            write_size = end_addr - addr + 1;
            for (size_t i = 0; i < write_size; i++) {

                write_buffer[i] = *((uint8_t *)(write_addr + i));
            }
            write_addr = (uint32_t)((uint32_t *)write_buffer);
        }

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, addr, write_addr) == HAL_OK) {

            for (uint8_t i = 0; i < write_size; i++) {
                if (*(uint8_t *)(addr + i) != *(uint8_t *)(write_addr + i)) {
                    result = -3;
                    goto __exit;
                }
            }
            addr += write_granularity;
            write_addr  += write_granularity;
        } else {
            result = -4;
            goto __exit;
        }
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
 */
int stm32_flash_erase(uint32_t addr, size_t size)
{
    int result = 0;
    uint32_t SECTORError = 0;

    uint32_t addr_bank1 = 0;
    uint32_t size_bank1 = 0;
#ifdef FLASH_BANK_2
    uint32_t addr_bank2 = 0;
    uint32_t size_bank2 = 0;
#endif

    if((addr + size) < FLASH_BANK2_BASE) {
        addr_bank1 = addr;
        size_bank1 = size;
#ifdef FLASH_BANK_2
        size_bank2 = 0;
#endif
    } else if(addr >= FLASH_BANK2_BASE) {
        size_bank1 = 0;
#ifdef FLASH_BANK_2
        addr_bank2 = addr;
        size_bank2 = size;
#endif
    } else {
        addr_bank1 = addr;
        size_bank1 = FLASH_BANK2_BASE - addr_bank1;
#ifdef FLASH_BANK_2
        addr_bank2 = FLASH_BANK2_BASE;
        size_bank2 = addr + size - FLASH_BANK2_BASE;
#endif
    }

    /*Variable used for Erase procedure*/
    FLASH_EraseInitTypeDef EraseInitStruct;
    /* Unlock the Flash to enable the flash control register access */
    HAL_FLASH_Unlock();
    EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
    SCB_DisableDCache();

    if(size_bank1) {
        EraseInitStruct.Sector    = (addr_bank1 - FLASH_BANK1_BASE) / FLASH_SECTOR_SIZE;
        EraseInitStruct.NbSectors = (addr_bank1 + size_bank1 -1 - FLASH_BANK1_BASE) / FLASH_SECTOR_SIZE - EraseInitStruct.Sector + 1;
        EraseInitStruct.Banks = FLASH_BANK_1;
        if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK) {
            result = -1;
            goto __exit;
        }
    }

#ifdef FLASH_BANK_2
    if(size_bank2) {
        EraseInitStruct.Sector    = (addr_bank2 - FLASH_BANK2_BASE) / FLASH_SECTOR_SIZE;
        EraseInitStruct.NbSectors = (addr_bank2 + size_bank2 -1 - FLASH_BANK2_BASE) / FLASH_SECTOR_SIZE - EraseInitStruct.Sector + 1;
        EraseInitStruct.Banks = FLASH_BANK_2;
        if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK) {
            result = -2;
            goto __exit;
        }
    }
#endif

__exit:

    SCB_EnableDCache();
    HAL_FLASH_Lock();

    if (result != 0) {
        return result;
    }

    return size;
}

