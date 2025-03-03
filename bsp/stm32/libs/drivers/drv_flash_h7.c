#include "drv_flash.h"

#if defined(DRV_BSP_H7)

static void drv_flash_unlock()
{
	HAL_FLASH_Unlock();
}

static void drv_flash_lock()
{
	HAL_FLASH_Lock();
}

static uint8_t drv_flash_get_sector(uint32_t addr)
{
	if (addr < 0x08020000)			return 0;
	else if (addr < 0x08040000)		return 1;
	else if (addr < 0x08060000)		return 2;
	else if (addr < 0x08080000)		return 3;
	else if (addr < 0x080A0000)		return 4;
	else if (addr < 0x080C0000)		return 5;
	else if (addr < 0x080E0000)		return 6; 
	else if (addr < 0x08100000)		return 7;
	else if (addr < 0x08120000)		return 8;
	else if (addr < 0x08140000)		return 9;
	else if (addr < 0x08160000)		return 10; 
	else if (addr < 0x08180000)		return 11;
	else if (addr < 0x081A0000)		return 12;
	else if (addr < 0x081C0000)		return 13; 
	else if (addr < 0x081E0000)		return 14;
	return 15;	
}

static uint8_t drv_flash_get_status(uint8_t bank_x)
{
	uint32_t status_reg;

	if (bank_x == 1)
		status_reg = FLASH->SR1;
	else if(bank_x == 2)
		status_reg = FLASH->SR2;

	if (status_reg & (1 << 17))			return 1;	//WRPERR=1, write protect error
	else if (status_reg & (1 << 18))	return 2;	//PGSERR=1, program sequence error
	else if (status_reg & (1 << 19))	return 3;	//STRBERR=1, strobe error
	else if (status_reg & (1 << 21))	return 4;	//INCERR=1, inconsistency error
	else if (status_reg & (1 << 22))	return 5;	//OPERR=1, write/erase error
	else if (status_reg & (1 << 23))	return 6;	//RDPERR=1, read protection error
	else if (status_reg & (1 << 24))	return 7;	//RDSERR=1, secure error
	else if (status_reg & (1 << 25))	return 8;	//SNECCERR=1, 1bit ecc error
	else if (status_reg & (1 << 26))	return 9;	//DBECCERR=1, 2bit ecc error

	return 0;
}

static uint8_t drv_flash_wait_handledone(uint8_t bank_x, uint32_t time)
{
	uint32_t status_reg = 0;
	uint32_t res = 0;
	
	if (bank_x == 1)
		status_reg = FLASH->SR1;
	else if(bank_x == 2)
		status_reg = FLASH->SR2;
	
	while (1) {
		if ((status_reg & 0X07) == 0)
			break;
        for (int i = 0; i < 4000*10; i++);
		time--;
		if (time == 0x00)
			return 0xFF;
		if (bank_x == 1)
			status_reg = FLASH->SR1;
		else if(bank_x == 2)
			status_reg = FLASH->SR2;
	}
	res = drv_flash_get_status(bank_x);
	/* clear error flag */
	if (res) {
		if (bank_x == 1)
			FLASH->CCR1 = 0X07EE0000;
		else if (bank_x == 2)
			FLASH->CCR2 = 0X07EE0000;
	}

	return res;
}

static uint8_t drv_flash_erase_sector(uint8_t sector_num)
{
	static int rescnt = 0;
	uint8_t res = 0;

	drv_flash_unlock();
	
	while (res && rescnt < 10000) {
		res = drv_flash_wait_handledone(sector_num / 8, 2000);
		rescnt++;
	}
	if (rescnt == 10000)
		return 0x11;
	
	if (res == 0) {
		if (sector_num < 8) {
			FLASH->CR1 &= ~(7 << 8);
			FLASH->CR1 &= ~(3 << 4);
			FLASH->CR1 |= (uint32_t)sector_num << 8;
			FLASH->CR1 |= 2 << 4;
			FLASH->CR1 |= 1 << 2;
			FLASH->CR1 |= 1 << 7;
		}else {
			FLASH->CR2 &= ~(7 << 8);
			FLASH->CR2 &= ~(3 << 4);
			FLASH->CR2 |= (uint32_t)(sector_num - 8) << 8;
			FLASH->CR2 |= 2 << 4;
			FLASH->CR2 |= 1 << 2;
			FLASH->CR2 |= 1 << 7;
		} 
		res = drv_flash_wait_handledone(sector_num / 8, 2000);
		if (sector_num < 8)
			FLASH->CR1 &= ~(1 << 2);
		else 
			FLASH->CR2 &= ~(1 << 2);
	}
	
	drv_flash_lock();
	return res;
}

static uint32_t drv_flash_read_word(uint32_t faddr)
{
	return *(volatile uint32_t *)faddr;
}

static uint8_t drv_flash_write_word(uint32_t faddr, uint32_t *pdata)
{
	uint8_t n_word = 8;
	uint8_t res = 0;
	uint8_t bank_x = 1;
	
	if (faddr < 0x08100000)
		bank_x = 1;
	else 
		bank_x = 2;
	
	res = drv_flash_wait_handledone(bank_x, 0XFF);	
	
	if (res == 0) {
		if (bank_x == 1) { 
			FLASH->CR1 &= ~(3 << 4);
			FLASH->CR1 |= 2 << 4;
			FLASH->CR1 |= 1 << 1; 
		}else if (bank_x == 2) {
			FLASH->CR2 &= ~(3 << 4);
			FLASH->CR2 |= 2 << 4;
			FLASH->CR2 |= 1 << 1;
		}
		__ISB();
		__DSB();
		
		while (n_word) {
			*(volatile uint32_t *)faddr = *pdata;
			faddr += 4;
			pdata++;
			n_word--;
		}
		__ISB();
		__DSB();
		
		res = drv_flash_wait_handledone(bank_x,0XFF);

		if (bank_x == 1)
			FLASH->CR1 &= ~(1 << 1);
		else if (bank_x == 2)
			FLASH->CR2 &= ~(1 << 1);
	}
	return res;
}


uint8_t drv_flash_write(uint32_t WriteAddr, const uint32_t *pBuffer, uint32_t NumToWrite)
{
	uint8_t res = 0;
	uint8_t status = 0;
	uint32_t addrx = 0;
	uint32_t endaddr = 0;	
    if (WriteAddr < STM32_FLASH_BASE_ADDR || WriteAddr % 32)
		return 0xFF;
	drv_flash_unlock();
	addrx = WriteAddr;
	endaddr = WriteAddr + NumToWrite * 4;
	if (addrx < 0X1FF00000) {
		while (addrx < endaddr) {
			if (drv_flash_read_word(addrx) != 0XFFFFFFFF) {   
				status = drv_flash_erase_sector(drv_flash_get_sector(addrx));
				if (status) {
					res = 0xFF;
					break;
				}
                SCB_CleanInvalidateDCache();
			}else {
				addrx+=4;
			}
		}
	}
	if (status == 0) {
		while (WriteAddr < endaddr) {
			if (drv_flash_write_word(WriteAddr,pBuffer)) {
				res = 0xFF;
				break;
			}
			WriteAddr += 32;
			pBuffer += 8;
		} 
	}
	drv_flash_lock();
	
	return res;
}

void drv_flash_read(uint32_t ReadAddr, uint32_t *pBuffer, uint32_t NumToRead)
{
	uint32_t i;
	for (i = 0; i < NumToRead; i++) {
		pBuffer[i] = drv_flash_read_word(ReadAddr);
		ReadAddr += 4;
	}
}

#endif // End With Define DRV_BSP_H7 
