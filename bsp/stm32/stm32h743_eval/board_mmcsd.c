#include "board_config.h"
#include <drv_mmcsd.h>

#ifdef CONFIG_BOARD_MMCSD_ENABLE

#ifdef CONFIG_BOARD_MMCSD_FATFS_SUPPORT
#include "ff.h"
#include "ff_drv.h"
const diskio_drv_ops_t mmcsd_driver;
static FATFS mmcsd_fatfs;
static char mmcsd_mnt_path[20];
#endif

/**
  bug:
  macro USE_SD_TRANSCEIVER in stm32xxx_hal_conf.h
  set USE_SD_TRANSCEIVER to 0, if it is 1, sd card initialization will start 1.8v process
 */
struct up_mmcsd_dev_s sd_dev = 
{
    .id = 1,
    .speed = 25 * 1000 * 1000, //25M
    .pin_clks = 1, //PC12
    .pin_d0 = 1, //PC8
    .pin_d1 = 1, //PC9
    .pin_d2 = 1, //PC10
    .pin_d3 = 1, //PC11
    .pin_cmd = 1, //PD2
    .priority = 4,
};

void board_mmcsd_init()
{
    int sdret = low_mmcsd_init(&sd_dev);

#ifdef CONFIG_BOARD_MMCSD_INFO_CHECK
    uint8_t tmp;
    tmp = low_mmcsd_waitrdy(&sd_dev);
    if (tmp != HAL_OK) {
        printf("\r\n[MMCSD] not ready\r\n");
    }
    low_mmcsd_getinfo(&sd_dev);

    printf("\r\n[MMCSD] init: %s \r\n", (sdret == 0) ? "success" : "failed");
    if (sdret != 0) {
        printf("[MMCSD] errorcode : %d \r\n", sd_dev.initret);
    }
    float total_capacity = 0;
    total_capacity = (float)(sd_dev.info.LogBlockNbr) / 1024.0f / 1024.0f / 1024.0f;
    printf("[MMCSD] total capacity : %.2f GB\r\n",(total_capacity * sd_dev.info.BlockSize));
    printf("[MMCSD] card block size : %d bytes\r\n",(sd_dev.info.BlockSize));
    switch (sd_dev.info.CardSpeed) {
    case CARD_NORMAL_SPEED:
        printf("[MMCSD] Normal Speed Card <12.5Mo/s , Spec Version 1.01 \r\n");
        break;
    case CARD_HIGH_SPEED:
        printf("[MMCSD] High Speed Card <25Mo/s , Spec version 2.00 \r\n");
        break;
    case CARD_ULTRA_HIGH_SPEED:
        printf("[MMCSD] UHS-I SD Card <50Mo/s for SDR50, DDR5 Cards and <104Mo/s for SDR104, Spec version 3.01 \r\n");
        break;	
    }
    switch (sd_dev.info.CardType) {
    case CARD_SDSC:
        printf("[MMCSD] SD Standard Capacity <2Go \r\n");
        break;
    case CARD_SDHC_SDXC:
        printf("[MMCSD] SD High Capacity <32Go, SD Extended Capacity <2To \r\n");
        break;
    case CARD_SECURED:
        printf("[MMCSD] SD Extended Capacity >2To \r\n");
        break;
    }
    switch (sd_dev.info.CardVersion) {
    case CARD_V1_X:
        printf("[MMCSD] card v1.x \r\n");
        break;
    case CARD_V2_X:
        printf("[MMCSD] card v2.x \r\n");
        break;
    }
#endif

#ifdef CONFIG_BOARD_MMCSD_FATFS_SUPPORT
    if (sdret == 0) {
        fatfs_link_drv(&mmcsd_driver, &mmcsd_mnt_path[0]);
        FRESULT ret_ff = f_mount(&mmcsd_fatfs, &mmcsd_mnt_path[0], 0);
        if (ret_ff != FR_OK) {
            // printf("[fat] mmcsd mount failed %d\r\n", ret_ff);
        }
    }
#endif

}

#ifdef CONFIG_BOARD_MMCSD_RAW_RW_TEST

#define NumOf_Blocks        64
#define Test_BlockSize     ((BLOCKSIZE * NumOf_Blocks) >> 2)
#define Test_Addr           0x00

static int32_t SD_Status;
static uint32_t SD_WriteBuffer[Test_BlockSize];
static uint32_t SD_ReadBuffer[Test_BlockSize];

void board_mmcsd_rw_test()
{
	uint32_t i = 0;	
	uint32_t ExecutionTime_Begin;
	uint32_t ExecutionTime_End;
	uint32_t ExecutionTime;
	float    ExecutionSpeed;

	ExecutionTime_Begin 	= HAL_GetTick();
	SD_Status = low_mmcsd_erase(&sd_dev, Test_Addr, Test_Addr+NumOf_Blocks);
    if (SD_Status == MSD_OK) {
        SD_Status = low_mmcsd_waitrdy(&sd_dev);
    } else {
        SD_Status += 100;
    }
	ExecutionTime_End		= HAL_GetTick();
	ExecutionTime = ExecutionTime_End - ExecutionTime_Begin;
	if (SD_Status == MSD_OK) {
		printf("\r\n[MMCSD] erase success, need time: %d ms\r\n",ExecutionTime);		
	} else {
		printf("\r\n[MMCSD] erase failed, error code: %d\r\n",SD_Status);
		while (1);
	}		

	for (i = 0; i < Test_BlockSize; i++) {
		SD_WriteBuffer[i] = i;
	}
	ExecutionTime_Begin 	= HAL_GetTick();
    SD_Status = low_mmcsd_writes(&sd_dev, SD_WriteBuffer, Test_Addr, NumOf_Blocks, RWPOLL);
    if (SD_Status == MSD_OK) {
        SD_Status = low_mmcsd_waitrdy(&sd_dev);
    } else {
        SD_Status += 100;
    }
	ExecutionTime_End		= HAL_GetTick();
	ExecutionTime  = ExecutionTime_End - ExecutionTime_Begin;
	ExecutionSpeed = (float)BLOCKSIZE * NumOf_Blocks / ExecutionTime /1024 ; // units in MB/S	
	if (SD_Status == MSD_OK) {
		printf("\r\n[MMCSD] write success, data: %dKB, need time: %dms, write speed: %.2f MB/s\r\n",
                BLOCKSIZE*NumOf_Blocks/1024,ExecutionTime,ExecutionSpeed);		
	} else {
		printf("\r\n[MMCSD] write failed, error code %d\r\n",SD_Status);
		while (1);
	}		

	ExecutionTime_Begin 	= HAL_GetTick();
    SD_Status = low_mmcsd_reads(&sd_dev, SD_ReadBuffer, Test_Addr, NumOf_Blocks, RWPOLL);
    if (SD_Status == MSD_OK) {
        SD_Status = low_mmcsd_waitrdy(&sd_dev);
    } else {
        SD_Status += 100;
    }
	ExecutionTime_End		= HAL_GetTick();
	ExecutionTime  = ExecutionTime_End - ExecutionTime_Begin;
	ExecutionSpeed = (float)BLOCKSIZE * NumOf_Blocks / ExecutionTime / 1024 ; 	// units in MB/S 
	
	if (SD_Status == MSD_OK ) {
		printf("\r\n[MMCSD] read success, data: %dKB, need time: %dms, read speed: %.2f MB/s\r\n",
                BLOCKSIZE * NumOf_Blocks/1024,ExecutionTime,ExecutionSpeed);		
	} else {
		printf("\r\n[MMCSD] read failed, error code %d\r\n",SD_Status);
		while (1);
	}		

	for (i = 0; i < Test_BlockSize; i++) {
		if (SD_WriteBuffer[i] != SD_ReadBuffer[i]) {
			printf("\r\n[MMCSD] data check failed\r\n");
			while(1);
		}
	}		
    printf("\r\n[MMCSD] rwtest+datacheck pass, mmcsd works\r\n");	
}

#endif // end with CONFIG_BOARD_MMCSD_RAW_RW_TEST

#ifdef CONFIG_BOARD_MMCSD_FATFS_SUPPORT

#define SD_DEFAULT_BLOCK_SIZE 512

#define ENABLE_SD_DMA_CACHE_MAINTENANCE_WRITE  0 
#define ENABLE_SD_DMA_CACHE_MAINTENANCE_READ   1                                                         

DSTATUS mmcsd_init(BYTE);
DSTATUS mmcsd_status(BYTE);
DRESULT mmcsd_read(BYTE, BYTE*, DWORD, UINT);
#if _USE_WRITE == 1
DRESULT mmcsd_write(BYTE, const BYTE*, DWORD, UINT);
#endif
#if _USE_IOCTL == 1
DRESULT mmcsd_ioctl(BYTE, BYTE, void*);
#endif

static volatile DSTATUS mmcsd_stat;
static volatile UINT write_stat = 0, read_stat = 0;
const diskio_drv_ops_t mmcsd_driver =
{
    mmcsd_init,
    mmcsd_status,
    mmcsd_read,
    #if _USE_WRITE == 1
    mmcsd_write,
    #endif
    #if _USE_IOCTL == 1
    mmcsd_ioctl,
    #endif
};

DSTATUS mmcsd_init(BYTE lun)
{
    uint8_t tmp;
    mmcsd_stat = STA_NOINIT;
    tmp = low_mmcsd_waitrdy(&sd_dev);
    if (tmp == HAL_OK) {
        mmcsd_stat &= ~STA_NOINIT;
    }
    return mmcsd_stat;
}

DSTATUS mmcsd_status(BYTE lun)
{
    uint8_t tmp;
    mmcsd_stat = STA_NOINIT;
    tmp = low_mmcsd_waitrdy(&sd_dev);
    if (tmp == HAL_OK) {
        mmcsd_stat &= ~STA_NOINIT;
    }
    return mmcsd_stat;
}

DRESULT mmcsd_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
	DRESULT res = RES_ERROR;
	int read_status;

    read_status = low_mmcsd_reads(&sd_dev, buff, sector, count, RWPOLL);
    if (read_status == MSD_OK) {
        read_status = low_mmcsd_waitrdy(&sd_dev);
    }

    return (read_status == MSD_OK) ? RES_OK : RES_ERROR;
}

#if _USE_WRITE == 1
DRESULT mmcsd_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
	DRESULT res = RES_ERROR;
	int write_status;

    write_status = low_mmcsd_writes(&sd_dev, buff, sector, count, RWPOLL);
    if (write_status == MSD_OK) {
        write_status = low_mmcsd_waitrdy(&sd_dev);
    }

    return (write_status == MSD_OK) ? RES_OK : RES_ERROR;
}
#endif

#if _USE_IOCTL == 1
DRESULT mmcsd_ioctl(BYTE lun, BYTE cmd, void *buff)
{
	DRESULT res = RES_ERROR;
	low_mmcsd_getinfo(&sd_dev);

	if (mmcsd_stat & STA_NOINIT) 
		return RES_NOTRDY;

	switch (cmd) {
	/* Make sure that no pending write process */
	case CTRL_SYNC :
		res = RES_OK;
		break;

	/* Get number of sectors on the disk (DWORD) */
	case GET_SECTOR_COUNT :
		*(DWORD*)buff = sd.info.LogBlockNbr;
		res = RES_OK;
		break;

	/* Get R/W sector size (WORD) */
	case GET_SECTOR_SIZE :
		*(WORD*)buff = sd.info.LogBlockSize;
		res = RES_OK;
		break;

	/* Get erase block size in unit of sector (DWORD) */
	case GET_BLOCK_SIZE :
		*(DWORD*)buff = sd.info.LogBlockSize / SD_DEFAULT_BLOCK_SIZE;
		res = RES_OK;
		break;

	default:
		res = RES_PARERR;
	}

	return res;
}
#endif
#endif // end with CONFIG_BOARD_MMCSD_FATFS_SUPPORT

#endif // end with CONFIG_BOARD_MMCSD_ENABLE