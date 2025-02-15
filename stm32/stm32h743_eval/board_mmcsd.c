#include "board_config.h"
#include <drv_sdmmc.h>

/**
  bug:
  macro USE_SD_TRANSCEIVER in stm32xxx_hal_conf.h
  set USE_SD_TRANSCEIVER to 0, if it is 1, sd card initialization will start 1.8v process
 */
struct drv_sdmmc_attr_t sd_attr;
struct drv_sdmmc_t sd;
void board_mmcsd_init()
{
    drv_sdmmc_attr_init(&sd_attr, 1, 25, /* io select */1, 1, 1, 1, 1, 1, /* priority */ 4);
    int sdret = drv_sdmmc_init(&sd, &sd_attr);

#ifdef BOARD_MTD_RW_TEST
    uint8_t tmp;
    tmp = drv_sdmmc_wait_ready(&sd);
    if (tmp != HAL_OK) {
        printf("\r\n[MMCSD] not ready\r\n");
    }
    drv_sdmmc_getinfo(&sd);

    printf("\r\n[MMCSD] init: %s \r\n", (sdret == 0) ? "success" : "failed");
    if (sdret != 0) {
        printf("[MMCSD] errorcode : %d \r\n", sd.initret);
    }
    float total_capacity = 0;
    total_capacity = (float)(sd.info.LogBlockNbr) / 1024.0f / 1024.0f / 1024.0f;
    printf("[MMCSD] total capacity : %.2f GB\r\n",(total_capacity * sd.info.BlockSize));
    printf("[MMCSD] card block size : %d bytes\r\n",(sd.info.BlockSize));
    switch (sd.info.CardSpeed) {
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
    switch (sd.info.CardType) {
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
    switch (sd.info.CardVersion) {
    case CARD_V1_X:
        printf("[MMCSD] card v1.x \r\n");
        break;
    case CARD_V2_X:
        printf("[MMCSD] card v2.x \r\n");
        break;
    }
#endif
}

#ifdef BOARD_MMCSD_RW_TEST

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
	SD_Status = drv_sdmmc_erase(&sd, Test_Addr, Test_Addr+NumOf_Blocks);
    if (SD_Status == MSD_OK) {
        SD_Status = drv_sdmmc_wait_ready(&sd); 
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
    SD_Status = drv_sdmmc_write_blocks(&sd, SD_WriteBuffer, Test_Addr, NumOf_Blocks, RWPOLL);
    if (SD_Status == MSD_OK) {
        SD_Status = drv_sdmmc_wait_ready(&sd); 
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
    SD_Status = drv_sdmmc_read_blocks(&sd, SD_ReadBuffer, Test_Addr, NumOf_Blocks, RWPOLL);
    if (SD_Status == MSD_OK) {
        SD_Status = drv_sdmmc_wait_ready(&sd); 
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

#endif

// bool sd_card_check(char* info_str)
// {
//     char str[300] = {0}, strtmp[100] = {0};
//     uint8_t tmp;

//     tmp = drv_sdmmc_wait_ready(&sd);
//     if (tmp != HAL_OK) {
//         sprintf(info_str,"not ready\r\n");
//         return false;
//     }
//     drv_sdmmc_getinfo(&sd);

//     switch (sd.initret) {
//     case HAL_ERROR:
//         sprintf(info_str,"sd init error\r\n");
//         return false;
//     case HAL_BUSY:
//         sprintf(info_str,"sd init error\r\n");
//         return false;
//     case HAL_TIMEOUT:
//         sprintf(info_str,"sd init timeout\r\n");
//         return false;
//     }

//     switch (tmp) {
//     case HAL_ERROR:
//         sprintf(info_str,"wait error\r\n");
//         return false;
//     case HAL_BUSY:
//         sprintf(info_str,"wait busy\r\n");
//         return false;
//     case HAL_TIMEOUT:
//         sprintf(info_str,"wait timeout\r\n");
//         return false;
//     }
    
//     sprintf(strtmp,"sd card ready\r\n");
//     strcat(str, strtmp);
//     float total_capacity = 0;
//     total_capacity = (float)(sd.info.LogBlockNbr) / 1024.0f / 1024.0f / 1024.0f;

//     sprintf(strtmp,"total capacity : %.2f GB\r\n",(total_capacity * sd.info.BlockSize));
//     strcat(str, strtmp);
//     sprintf(strtmp,"card block size : %d bytes\r\n",(sd.info.BlockSize));
//     strcat(str, strtmp);
    
//     switch (sd.info.CardSpeed) {
//     case CARD_NORMAL_SPEED:
//         sprintf(strtmp,"Normal Speed Card <12.5Mo/s , Spec Version 1.01 \r\n");
//         break;
//     case CARD_HIGH_SPEED:
//         sprintf(strtmp,"High Speed Card <25Mo/s , Spec version 2.00 \r\n");
//         break;
//     case CARD_ULTRA_HIGH_SPEED:
//         sprintf(strtmp,"UHS-I SD Card <50Mo/s for SDR50, DDR5 Cards and <104Mo/s for SDR104, Spec version 3.01 \r\n");
//         break;	
//     }
//     strcat(str, strtmp);
    
//     switch (sd.info.CardType) {
//     case CARD_SDSC:
//         sprintf(strtmp,"SD Standard Capacity <2Go \r\n");
//         break;
//     case CARD_SDHC_SDXC:
//         sprintf(strtmp,"SD High Capacity <32Go, SD Extended Capacity <2To \r\n");
//         break;
//     case CARD_SECURED:
//         sprintf(strtmp,"SD Extended Capacity >2To \r\n");
//         break;
//     }
//     strcat(str, strtmp);
    
//     switch (sd.info.CardVersion) {
//     case CARD_V1_X:
//         sprintf(strtmp,"card v1.x \r\n");
//         break;
//     case CARD_V2_X:
//         sprintf(strtmp,"card v2.x \r\n");
//         break;
//     }
//     strcat(str, strtmp);
    
//     strcpy(info_str,str);
//     return true;
// }
