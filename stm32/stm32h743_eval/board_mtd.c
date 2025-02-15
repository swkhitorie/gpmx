#include "board_config.h"
#include <drv_qspi.h>

#ifdef BOARD_MTD_QSPIFLASH_FATFS_SUPPORT
#include "ff_gen_drv.h"
const diskio_drv_ops_t mtd_driver;
static char mnt_mnt_path[20];
#endif

void board_mtd_init()
{
    //qspi control size: 2^(22 + 1)
    drv_qspi_init(1, 2, 22, /* io select */ 1, 2, 2, 1, 2, 2);
    int ret1 = w25qxx_init();
    int id = w25qxx_readid();
#ifdef BOARD_MTD_RW_TEST
    printf("\r\n[Flash] qspi flash init: %s, id: %X\r\n", 
		(ret1 == 1) ? "success" : "failed", id);
#endif
#ifdef BOARD_MTD_QSPIFLASH_FATFS_SUPPORT
	if (ret1 == 1) {
		fatfs_link_drv(&mtd_driver, &mnt_mnt_path[0]);
	}
#endif
}

#ifdef BOARD_MTD_RW_TEST
#define W25Qxx_NumByteToTest (32*1024)	
static uint32_t w25qxx_test_addr = 0;
static uint8_t  w25qxx_wbuffer[W25Qxx_NumByteToTest];
static uint8_t  w25qxx_rbuffer[W25Qxx_NumByteToTest];
int32_t qspi_status;

void board_mtd_rw_test()
{
    uint32_t i = 0;
    uint32_t ExecutionTime_Begin;
    uint32_t ExecutionTime_End;
    uint32_t ExecutionTime;
    float    ExecutionSpeed;

	ExecutionTime_Begin 	= HAL_GetTick();
	qspi_status 			= w25qxx_blockerase_32k(w25qxx_test_addr);
	ExecutionTime_End		= HAL_GetTick();
	ExecutionTime = ExecutionTime_End - ExecutionTime_Begin;
	if (qspi_status == QSPI_W25QXX_OK) {
		printf("\r\n[Flash] w25qxx erase success(32k), execution time: %d ms\r\n", ExecutionTime);		
	} else {
		printf("\r\n[Flash] w25qxx erase failed, error code: %d\r\n",qspi_status);
		while(1);
	}	
	

	for (i = 0; i < W25Qxx_NumByteToTest; i++) {
		w25qxx_wbuffer[i] = i;
	}
	ExecutionTime_Begin 	= HAL_GetTick();
	qspi_status				= w25qxx_writebuffer(w25qxx_wbuffer, w25qxx_test_addr, W25Qxx_NumByteToTest);
	ExecutionTime_End		= HAL_GetTick();
	ExecutionTime  = ExecutionTime_End - ExecutionTime_Begin;
	ExecutionSpeed = (float)W25Qxx_NumByteToTest/1024/ExecutionTime*1000; // unit in KB/S
	if (qspi_status == QSPI_W25QXX_OK ) {
		printf("\r\n[Flash] w25qxx write success, data: %d KB, need time: %d MS, write speed: %.2f KB/s\r\n", 
                W25Qxx_NumByteToTest/1024,ExecutionTime,ExecutionSpeed);		
	} else {
		printf("\r\n[Flash] w25qxx write failed, error code: %d\r\n",qspi_status);
		while (1);
	}		

	ExecutionTime_Begin 	= HAL_GetTick();
	qspi_status				= w25qxx_readbuffer(w25qxx_rbuffer, w25qxx_test_addr, W25Qxx_NumByteToTest);
	ExecutionTime_End		= HAL_GetTick();
	ExecutionTime  = ExecutionTime_End - ExecutionTime_Begin;
	ExecutionSpeed = (float)W25Qxx_NumByteToTest/1024/1024 /ExecutionTime*1000 ; 	// unit in MB/S 
	
	if (qspi_status == QSPI_W25QXX_OK) {
		if (ExecutionTime == 0) {
			printf ("\r\n[Flash] w25qxx read success, data: %d KB, need time: <1 MS, read speed: %.2f MB/s\r\n",
                W25Qxx_NumByteToTest/1024,ExecutionTime,(float)W25Qxx_NumByteToTest/1024/1024 /1*1000);	
		} else {
			printf ("\r\n[Flash] w25qxx read success, data: %d KB, need time: %d MS, read speed: %.2f MB/s\r\n",
                W25Qxx_NumByteToTest/1024,ExecutionTime,ExecutionSpeed);	
		}
	} else {
		printf("\r\n[Flash] w25qxx read failed, error code: %d\r\n",qspi_status);
		while (1);
	}			


	for (i = 0; i < W25Qxx_NumByteToTest; i++) {
		if (w25qxx_wbuffer[i] != w25qxx_rbuffer[i]) {
			printf ("\r\n[Flash] w25qxx rw test datacheck failed!!! \r\n");	
			while(1);
		}
	}			
	printf ("\r\n[Flash] w25qxx rw test datacheck pass, qspi driver works\r\n");		

	printf ("\r\n[Flash] start read w25qxx whole chip\r\n");		
	ExecutionTime_Begin 	= HAL_GetTick();	
	for (i = 0; i < W25QXX_FLASHSIZE/(W25Qxx_NumByteToTest); i++) {
		qspi_status		 = w25qxx_readbuffer(w25qxx_rbuffer, w25qxx_test_addr, W25Qxx_NumByteToTest);
		w25qxx_test_addr = w25qxx_test_addr + W25Qxx_NumByteToTest;	
	}

	ExecutionTime_End		= HAL_GetTick();
	ExecutionTime  = ExecutionTime_End - ExecutionTime_Begin;
	ExecutionSpeed = (float)W25QXX_FLASHSIZE/1024/1024 /ExecutionTime*1000  ;

	if (qspi_status == QSPI_W25QXX_OK) {
		printf ("\r\n[Flash] w25qxx read success, data: %d KB, need time: %d MS, read speed: %.2f MB/s\r\n",
            W25Qxx_NumByteToTest/1024,ExecutionTime,ExecutionSpeed);	
	} else {
		printf("\r\n[Flash] w25qxx read failed, error code: %d\r\n",qspi_status);
		while (1);
	}	

    printf("\r\n[Flash] w25qxx rwtest pass\r\n");
}
#endif

#ifdef BOARD_MTD_QSPIFLASH_FATFS_SUPPORT

DSTATUS mtd_init(BYTE);
DSTATUS mtd_status(BYTE);
DRESULT mtd_read(BYTE, BYTE*, DWORD, UINT);
#if _USE_WRITE == 1
DRESULT mtd_write(BYTE, const BYTE*, DWORD, UINT);
#endif
#if _USE_IOCTL == 1
DRESULT mtd_ioctl(BYTE, BYTE, void*);
#endif

static volatile DSTATUS mtd_stat = STA_NOINIT;
const diskio_drv_ops_t mtd_driver =
{
	mtd_init,
    mtd_status,
    mtd_read,
#if _USE_WRITE == 1
    mtd_write,
#endif
#if _USE_IOCTL == 1
    mtd_ioctl,
#endif
};

DSTATUS mtd_init(BYTE lun)
{
	mtd_stat = RES_OK;
	return mtd_stat;
}

DSTATUS mtd_status(BYTE lun)
{
	mtd_stat = RES_OK;
	return mtd_stat;
}

DRESULT mtd_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
	DRESULT res = RES_OK;
    int qspi_status = w25qxx_readbuffer(buff, sector << 12, count << 12);
	if (qspi_status != QSPI_W25QXX_OK) {
        res = RES_ERROR;
	}
	return res;
}

#if _USE_WRITE == 1
DRESULT mtd_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
	DRESULT res = RES_OK;
	uint32_t addr;
	uint16_t i;
	int qspi_status;
	BYTE *p;
	p = (BYTE *)buff;
	for (i = 0; i < count; i++) {
		addr = (sector + i) << 12;
		qspi_status = w25qxx_sector_erase(addr);
		if (qspi_status == QSPI_W25QXX_OK) {
			res = RES_ERROR;
			goto mtd_write_end;
		}
		for (int j = 0; j < 16; j++) {
            qspi_status = w25qxx_writebuffer(p + 256*j, addr + 256*j, 256);
			if (qspi_status == QSPI_W25QXX_OK) {
				res = RES_ERROR;
				goto mtd_write_end;
			}
		}
		p += 4096;					
	}
mtd_write_end:
	return res;
}
#endif

#if _USE_IOCTL == 1
DRESULT mtd_ioctl(BYTE lun, BYTE cmd, void *buff)
{
	DRESULT res = RES_ERROR;

	switch (cmd) {
	/* Make sure that no pending write process */
	case CTRL_SYNC :
	res = RES_OK;
	break;

	/* Get number of sectors on the disk (DWORD) */
	case GET_SECTOR_COUNT :
	*(DWORD*)buff = 1024 * 5;
	res = RES_OK;
	break;

	/* Get R/W sector size (WORD) */
	case GET_SECTOR_SIZE :
	*(WORD*)buff = 4096;
	res = RES_OK;
	break;

	/* Get erase block size in unit of sector (DWORD) */
	case GET_BLOCK_SIZE :
	  res = RES_OK;
	break;

	default:
	res = RES_PARERR;
	}

	return res;
}
#endif
#endif
