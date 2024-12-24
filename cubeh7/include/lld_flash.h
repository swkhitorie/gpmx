#ifndef __LLD_FLASH_H_
#define __LLD_FLASH_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * low level driver for stm32h7 series, base on cubehal library
 * module cpu flash
*/

#include "lld_h7_global.h"

#define NOR_FLASH_BASE_ADDR		(0x08000000)
#define NOR_FLASH_END_ADDR		(0x081FFFFF)
#define NOR_FLASH_WAIT_TIME		(50000)
#define NOR_FLASH_PROGRAMM_WORD	(8)

/**
 * @brief Flash Features -->
 *		  max memory capacity is 2MB
 *		  h7 flash is 266bits word memory, each word includes -->
 *		  one flash word( 8 memory word, or 32 bytes, 256bits)
 *		  10 bits of ECC(error code correction)
 *		  only support flash program at 256 bits 
 *		  the times of sector erase in h7 board is 950ms
 *		  the times of single word programm in h7 board is 190us
	|------bank------|--------name----------|------------- Flash start address ---------|--- size ---| 
	|				 |		Sector0			|			0x0800 0000	-- 0x0801 FFFF		|	  128K   |
	|				 |		Sector1			|			0x0802 0000 -- 0x0803 FFFF		|	  128K   |	
	|				 |		Sector2			|			0x0804 0000 -- 0x0805 FFFF		|	  128K   |
	|				 |		Sector3			|			0x0806 0000	-- 0x0807 FFFF		|	  128K   |
	|	BANK1	 	 |		Sector4			|			0x0808 0000 -- 0x0809 FFFF		|	  128K   |	
	|				 |		Sector5			|			0x080A 0000	-- 0x080B FFFF		|	  128K   |
	|				 |		Sector6			|			0x080C 0000 -- 0x080D FFFF		|	  128K   |	
	|				 |		Sector7			|			0x080E 0000	-- 0x080F FFFF		|	  128K   |
	|				 |	System memory 		|			0x1FF0 0000 -- 0x1FF1 FFFF		|	  128K   |	
	|================================================================================================|
	|				 |		Sector0			|			0x0810 0000	-- 0x0811 FFFF		|	  128K   |
	|				 |		Sector1			|			0x0812 0000 -- 0x0813 FFFF		|	  128K   |	
	|				 |		Sector2			|			0x0814 0000 -- 0x0815 FFFF		|	  128K   |
	|				 |		Sector3			|			0x0816 0000	-- 0x0817 FFFF		|	  128K   |
	|	BANK2		 |		Sector4			|			0x0818 0000 -- 0x0819 FFFF		|	  128K   |	
	|				 |		Sector5			|			0x081A 0000	-- 0x081B FFFF		|	  128K   |
	|				 |		Sector6			|			0x081C 0000 -- 0x081D FFFF		|	  128K   |	
	|				 |		Sector7			|			0x081E 0000	-- 0x081F FFFF		|	  128K   |
	|				 |	System memory 		|			0x1FF4 0000 -- 0x1FF5 FFFF		|	  128K   |	
	|================================================================================================|
*/

uint8_t lld_flash_write(uint32_t WriteAddr, uint32_t *pBuffer, uint32_t NumToWrite);
void lld_flash_read(uint32_t ReadAddr, uint32_t *pBuffer, uint32_t NumToRead);

#ifdef __cplusplus
}
#endif

#endif
