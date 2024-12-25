/**
 * low level driver for stm32f4 series, base on std periph library
 * module core flash
*/

#include "lld_flash.h"

static uint32_t lld_flash_readword(uint32_t addr) { return *(volatile uint32_t*)addr; }
static uint32_t lld_flash_get_sector(uint32_t addr)
{
    uint32_t sector = 0;
    if((addr < ADDR_FLASH_SECTOR_1) && (addr >= ADDR_FLASH_SECTOR_0)) { 
        sector = FLASH_Sector_0; 
    } else if((addr < ADDR_FLASH_SECTOR_2) && (addr >= ADDR_FLASH_SECTOR_1)) {
        sector = FLASH_Sector_1; 
    } else if((addr < ADDR_FLASH_SECTOR_3) && (addr >= ADDR_FLASH_SECTOR_2)) {
        sector = FLASH_Sector_2; 
    } else if((addr < ADDR_FLASH_SECTOR_4) && (addr >= ADDR_FLASH_SECTOR_3)) {
        sector = FLASH_Sector_3; 
    } else if((addr < ADDR_FLASH_SECTOR_5) && (addr >= ADDR_FLASH_SECTOR_4)) {
        sector = FLASH_Sector_4; 
    } else if((addr < ADDR_FLASH_SECTOR_6) && (addr >= ADDR_FLASH_SECTOR_5)) {
        sector = FLASH_Sector_5; 
    } else if((addr < ADDR_FLASH_SECTOR_7) && (addr >= ADDR_FLASH_SECTOR_6)) {
        sector = FLASH_Sector_6; 
    } else if((addr < ADDR_FLASH_SECTOR_8) && (addr >= ADDR_FLASH_SECTOR_7)) {
        sector = FLASH_Sector_7; 
    } else if((addr < ADDR_FLASH_SECTOR_9) && (addr >= ADDR_FLASH_SECTOR_8)) {
        sector = FLASH_Sector_8; 
    } else if((addr < ADDR_FLASH_SECTOR_10) && (addr >= ADDR_FLASH_SECTOR_9)) {
        sector = FLASH_Sector_9; 
    } else if((addr < ADDR_FLASH_SECTOR_11) && (addr >= ADDR_FLASH_SECTOR_10)) {
        sector = FLASH_Sector_10; 
    } else { sector = FLASH_Sector_11; }
    
    return sector;
}

void lld_flash_write(uint32_t addr, const uint32_t *p, uint32_t size)
{
	FLASH_Status status = FLASH_COMPLETE;
	uint32_t addrx = 0;
	uint32_t endaddr = 0;
	if (addr < STM32_FLASH_BASE || addr % 4) return;
	FLASH_Unlock();
	FLASH_DataCacheCmd(DISABLE);
	
	addrx = addr;
	endaddr = addr + size * 4;
	if (addrx < 0x1FFF0000) {
		while (addrx < endaddr) {
			if (lld_flash_readword(addrx) != 0xFFFFFFFF) {
				status = FLASH_EraseSector(lld_flash_get_sector(addrx), VoltageRange_3);
				if (status != FLASH_COMPLETE) break;
			}else {
				addrx += 4;
            }
		}
	}
    
	if (status == FLASH_COMPLETE) {
		while (addr < endaddr) {
			if (FLASH_ProgramWord(addr, *p) != FLASH_COMPLETE) {
				break;
			}
			addr += 4;
			p++;
		}
	}
    
	FLASH_DataCacheCmd(ENABLE);
	FLASH_Lock();
}

void lld_flash_read(uint32_t addr, uint32_t *p, uint32_t size)
{
	uint32_t i;
	for (i = 0; i < size; i++) {
		p[i] = lld_flash_readword(addr);
		addr += 4;
	}
}
