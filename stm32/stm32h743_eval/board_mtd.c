#include "board_config.h"
#include <drv_qspi.h>

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

