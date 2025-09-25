#include "drv_mmcsd.h"

#define   SD_TRANSFER_OK                ((uint8_t)0x00)
#define   SD_TRANSFER_BUSY              ((uint8_t)0x01)

#define   MSD_OK                        ((uint8_t)0x00)
#define   MSD_ERROR                     ((uint8_t)0x01)
#define   MSD_ERROR_SD_NOT_PRESENT      ((uint8_t)0x02)
#define   SD_TIMEOUT				    (5 * 1000)

#if defined(DRV_STM32_F1) && defined(SDIO)
#define CONFIG_STM32_MMCSD_NUM (1)
#elif defined(DRV_STM32_F4) && defined(SDIO)
#define CONFIG_STM32_MMCSD_NUM (1)
#elif defined(DRV_STM32_H7)
#define CONFIG_STM32_MMCSD_NUM (2)
#else
#error unknown config in stm32_mmcsd
#endif

struct stm32_mmcsd_dev_s
{
    uint32_t speed;    // 50M ~ 100/200M...
    uint8_t controller;
    uint8_t priority;

    bool isinitalized;
    int  initialret;

    SD_HandleTypeDef       handle;
	HAL_SD_CardCIDTypedef  cid;
	HAL_SD_CardCSDTypedef  csd;
	HAL_SD_CardInfoTypeDef info;

} mmcsd_list[CONFIG_STM32_MMCSD_NUM];

static void _mmcsd_clk_init(int controller);
static void _mmcsd_irq_cfg(int controller, uint8_t priority);
static int  _mmcsd_init(int controller);

static int  stm32_mmcsd_waitrdy(struct stm32_mmcsd_dev_s *dev);
static void stm32_mmcsd_get_cardinfo(struct stm32_mmcsd_dev_s *dev);
static int  stm32_mmcsd_get_state(struct stm32_mmcsd_dev_s *dev);
static int  stm32_mmcsd_erase(struct stm32_mmcsd_dev_s *dev, uint32_t start, uint32_t end);
static int  stm32_mmcsd_reads(struct stm32_mmcsd_dev_s *dev, uint32_t *p, uint32_t addr, uint32_t num, int way);
static int  stm32_mmcsd_writes(struct stm32_mmcsd_dev_s *dev, uint32_t *p, uint32_t addr, uint32_t num, int way);

/****************************************************************************
 * Private Function
 ****************************************************************************/
void _mmcsd_clk_init(int controller)
{
#if defined(DRV_STM32_F1) || defined(DRV_STM32_F4)
    ((void)controller);
    __HAL_RCC_SDIO_CLK_ENABLE();
#endif

#if defined(DRV_STM32_H7)
    switch (controller) {
    case 1: 
        __HAL_RCC_SDMMC1_CLK_ENABLE();
        __HAL_RCC_SDMMC1_FORCE_RESET();
        __HAL_RCC_SDMMC1_RELEASE_RESET();
        break;
    case 2: 
        __HAL_RCC_SDMMC2_CLK_ENABLE();
        __HAL_RCC_SDMMC2_FORCE_RESET();
        __HAL_RCC_SDMMC2_RELEASE_RESET();
        break;
    }
#endif
}

void _mmcsd_irq_cfg(int controller, uint8_t priority)
{
#if defined(DRV_STM32_F1) || defined(DRV_STM32_F4)
    HAL_NVIC_SetPriority(SDIO_IRQn, priority, 0);
    HAL_NVIC_EnableIRQ(SDIO_IRQn);
#endif

#if defined(DRV_STM32_H7)
    uint32_t sd_irq[2] = {SDMMC1_IRQn, SDMMC2_IRQn};

    HAL_NVIC_SetPriority(sd_irq[controller-1], priority, 0);
    HAL_NVIC_EnableIRQ(sd_irq[controller-1]);
#endif
}

int _mmcsd_init(int controller)
{
    int ret = 0;

    struct stm32_mmcsd_dev_s *immc = &mmcsd_list[controller-1];
#if defined(DRV_STM32_H7)
    int idx = controller;

    RCC_PeriphCLKInitTypeDef sd_clk = {0};
    sd_clk.PeriphClockSelection     = RCC_PERIPHCLK_SDMMC;
    sd_clk.SdmmcClockSelection      = RCC_SDMMCCLKSOURCE_PLL;
    HAL_RCCEx_PeriphCLKConfig(&sd_clk);

    immc->handle.Instance                 = (controller==1) ? SDMMC1 : SDMMC2;
    immc->handle.Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    immc->handle.Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
    immc->handle.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    immc->handle.Init.BusWide             = SDMMC_BUS_WIDE_4B;
    immc->handle.Init.ClockDiv            = 6;
    ret = HAL_SD_Init(&immc->handle);
#endif

#if defined(DRV_STM32_F1) || defined(DRV_STM32_F4)
    immc->handle.Instance = SDIO;
    immc->handle.Init.ClockEdge           = SDIO_CLOCK_EDGE_RISING;
    immc->handle.Init.ClockBypass         = SDIO_CLOCK_BYPASS_DISABLE;
    immc->handle.Init.ClockPowerSave      = SDIO_CLOCK_POWER_SAVE_DISABLE;
    immc->handle.Init.BusWide             = SDIO_BUS_WIDE_1B;
    immc->handle.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
    immc->handle.Init.ClockDiv            = 0;
    ret = HAL_SD_Init(&immc->handle);

    if (ret == HAL_OK) {
        HAL_SD_ConfigWideBusOperation(&immc->handle, SDIO_BUS_WIDE_4B);
    }
#endif

    immc->initialret = ret;

    if (ret == HAL_OK) {
        immc->isinitalized = true;
    } else {
        immc->isinitalized = false;
    }
    return ret;
}

int stm32_mmcsd_waitrdy(struct stm32_mmcsd_dev_s *dev)
{
	uint32_t loop = SD_TIMEOUT;

	/* Wait for the Erasing process is completed */
	/* Verify that SD card is ready to use after the Erase */
	while (loop > 0) {
		loop--;
		if (HAL_SD_GetCardState(&dev->handle) == HAL_SD_CARD_TRANSFER) {
			return MSD_OK;
        }
	}

	return MSD_ERROR;
}

void stm32_mmcsd_get_cardinfo(struct stm32_mmcsd_dev_s *dev)
{
    HAL_SD_GetCardInfo(&dev->handle, &dev->info);
}

int stm32_mmcsd_get_state(struct stm32_mmcsd_dev_s *dev)
{
    int state;
    state = HAL_SD_GetCardState(&dev->handle);

    return ((state == HAL_SD_CARD_TRANSFER) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY);
}

int stm32_mmcsd_erase(struct stm32_mmcsd_dev_s *dev, uint32_t start, uint32_t end)
{
    int ret;
    ret = HAL_SD_Erase(&dev->handle, start, end);
    return (ret == HAL_OK) ? MSD_OK : MSD_ERROR;
}

int stm32_mmcsd_reads(struct stm32_mmcsd_dev_s *dev, uint32_t *p, uint32_t addr, uint32_t num, int way)
{
    int ret;
    switch (way) {
    case 0:
        ret = (HAL_SD_ReadBlocks(&dev->handle, (uint8_t *)p, addr, num, 100U*num) == HAL_OK) ? MSD_OK : MSD_ERROR;
        break;
    case 1:
        ret = (HAL_SD_ReadBlocks_DMA(&dev->handle, (uint8_t *)p, addr, num) == HAL_OK) ? MSD_OK : MSD_ERROR;
        break;
    }
    return ret;
}

int stm32_mmcsd_writes(struct stm32_mmcsd_dev_s *dev, uint32_t *p, uint32_t addr, uint32_t num, int way)
{
    int ret;
    switch (way) {
    case 0:
        ret = (HAL_SD_WriteBlocks(&dev->handle, (uint8_t *)p, addr, num, 100U*num) == HAL_OK) ? MSD_OK : MSD_ERROR;
        break;
    case 1:
        ret = (HAL_SD_WriteBlocks_DMA(&dev->handle, (uint8_t *)p, addr, num) == HAL_OK) ? MSD_OK : MSD_ERROR;
        break;
    }
    return ret;
}

/****************************************************************************
 * Public Function Interface 
 ****************************************************************************/

int hw_stm32_mmcsd_init(int controller, uint32_t speed, uint8_t priority)
{
    int ret = 0;

    _mmcsd_clk_init(controller);

    _mmcsd_irq_cfg(controller, priority);

    ret = _mmcsd_init(controller);

    return ret;
}

void hw_stm32_mmcsd_info(int controller)
{
    struct stm32_mmcsd_dev_s *mmc = &mmcsd_list[controller-1];
    float total_capacity = 0;
    int tmp;

    MMCSD_INFO("\r\n");

    tmp = stm32_mmcsd_waitrdy(mmc);
    if (tmp != MSD_OK) {
        MMCSD_INFO("\r\n[mmcsd] not ready\r\n");
        return;
    }

    stm32_mmcsd_get_cardinfo(mmc);

    if (mmc->initialret != 0) {
        MMCSD_INFO("[mmcsd] init failed, errorcode : %d \r\n", mmc->initialret);
        return;
    }

    total_capacity = (float)(mmc->info.LogBlockNbr) / 1024.0f / 1024.0f / 1024.0f;
    MMCSD_INFO("[mmcsd] block sz: %db, capacity: %.2fGB\r\n",
        mmc->info.BlockSize, total_capacity * mmc->info.BlockSize);

    // switch (mmc->info.CardSpeed) {
    // case CARD_NORMAL_SPEED:
    //     MMCSD_INFO("[mmcsd] Normal Speed Card <12.5Mo/s , Spec Version 1.01 \r\n");
    //     break;
    // case CARD_HIGH_SPEED:
    //     MMCSD_INFO("[mmcsd] High Speed Card <25Mo/s , Spec version 2.00 \r\n");
    //     break;
    // case CARD_ULTRA_HIGH_SPEED:
    //     MMCSD_INFO("[mmcsd] UHS-I SD Card <50Mo/s for SDR50, DDR5 Cards and <104Mo/s for SDR104, Spec version 3.01 \r\n");
    //     break;	
    // }

    switch (mmc->info.CardType) {
    case CARD_SDSC:
        MMCSD_INFO("[mmcsd] Standard Capacity <2Go \r\n");
        break;
    case CARD_SDHC_SDXC:
        MMCSD_INFO("[mmcsd] High Capacity <32Go, SD Extended Capacity <2To \r\n");
        break;
    case CARD_SECURED:
        MMCSD_INFO("[mmcsd] Extended Capacity >2To \r\n");
        break;
    }

    switch (mmc->info.CardVersion) {
    case CARD_V1_X:
        MMCSD_INFO("[mmcsd] v1.x \r\n");
        break;
    case CARD_V2_X:
        MMCSD_INFO("[mmcsd] v2.x \r\n");
        break;
    }
}


/****************************************************************************
 * STM32 HAL Library Callback 
 ****************************************************************************/
__weak void BSP_SD_AbortCallback(void)
{

}

__weak void BSP_SD_WriteCpltCallback(void)
{

}

__weak void BSP_SD_ReadCpltCallback(void)
{

}

__weak void BSP_SD_ErrorCallback(void)
{

}

__weak void BSP_SD_DriveTransciver_1_8V_Callback(FlagStatus status)
{

}

void HAL_SD_AbortCallback(SD_HandleTypeDef *hsd)
{
    BSP_SD_AbortCallback();
}

void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
    BSP_SD_WriteCpltCallback();
}

void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
    BSP_SD_ReadCpltCallback();
}

void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd)
{
    BSP_SD_ErrorCallback();
}

void HAL_SD_DriveTransciver_1_8V_Callback(FlagStatus status)
{
    BSP_SD_DriveTransciver_1_8V_Callback(status);
}

/****************************************************************************
 * STM32 IRQ
 ****************************************************************************/
void SDMMC1_IRQHandler(void)
{
    HAL_SD_IRQHandler(&mmcsd_list[0].handle);
}

#if defined(DRV_STM32_H7)
void SDMMC2_IRQHandler(void)
{
    HAL_SD_IRQHandler(&mmcsd_list[1].handle);
}
#endif

/****************************************************************************
 * FatFS Support
 ****************************************************************************/
#ifdef CONFIG_STM32_MMCSD_FATFS_SUPPORT
#include "ff.h"
#include "ff_drv.h"

#define SD_DEFAULT_BLOCK_SIZE                  512
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

struct stm32_mmcsd_dev_s *fs_mmc;
const  diskio_drv_ops_t   mmcsd_driver;
static FATFS              mmcsd_fatfs;
static char               mmcsd_mnt_path[20];

static volatile DSTATUS   mmcsd_stat;
static volatile UINT      write_stat = 0, read_stat = 0;
const diskio_drv_ops_t    mmcsd_driver =
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
#include <fatfs_test.h>
void hw_stm32_mmcsd_fs_init(int controller)
{
    fs_mmc = &mmcsd_list[controller-1];
    fatfs_link_drv(&mmcsd_driver, &mmcsd_mnt_path[0]);
    FRESULT ret_ff = f_mount(&mmcsd_fatfs, &mmcsd_mnt_path[0], 0);
    if (ret_ff != FR_OK) {
        MMCSD_INFO("[fat] mmcsd mount failed %d\r\n", ret_ff);
        return;
    }

    // ff_ls(mmcsd_mnt_path);
}

DSTATUS mmcsd_init(BYTE lun)
{
    uint8_t tmp;
    mmcsd_stat = STA_NOINIT;
    tmp = stm32_mmcsd_waitrdy(fs_mmc);
    if (tmp == MSD_OK) {
        mmcsd_stat &= ~STA_NOINIT;
    }
    return mmcsd_stat;
}

DSTATUS mmcsd_status(BYTE lun)
{
    uint8_t tmp;
    mmcsd_stat = STA_NOINIT;
    tmp = stm32_mmcsd_waitrdy(fs_mmc);
    if (tmp == MSD_OK) {
        mmcsd_stat &= ~STA_NOINIT;
    }
    return mmcsd_stat;
}

DRESULT mmcsd_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
	DRESULT res = RES_ERROR;
	int read_status;

    read_status = stm32_mmcsd_reads(fs_mmc, (uint32_t *)buff, sector, count, 0);   // blocking mode
    if (read_status == MSD_OK) {
        read_status = stm32_mmcsd_waitrdy(fs_mmc);
    }

    return (read_status == MSD_OK) ? RES_OK : RES_ERROR;
}

#if _USE_WRITE == 1
DRESULT mmcsd_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
	DRESULT res = RES_ERROR;
	int write_status;

    write_status = stm32_mmcsd_writes(fs_mmc, (uint32_t *)buff, sector, count, 0);   // blocking mode
    if (write_status == MSD_OK) {
        write_status = stm32_mmcsd_waitrdy(fs_mmc);
    }

    return (write_status == MSD_OK) ? RES_OK : RES_ERROR;
}
#endif

#if _USE_IOCTL == 1
DRESULT mmcsd_ioctl(BYTE lun, BYTE cmd, void *buff)
{
	DRESULT res = RES_ERROR;
	stm32_mmcsd_get_cardinfo(fs_mmc);

	if (mmcsd_stat & STA_NOINIT) 
		return RES_NOTRDY;

	switch (cmd) {
	/* Make sure that no pending write process */
	case CTRL_SYNC :
		res = RES_OK;
		break;

	/* Get number of sectors on the disk (DWORD) */
	case GET_SECTOR_COUNT :
		*(DWORD*)buff = fs_mmc->info.LogBlockNbr;
		res = RES_OK;
		break;

	/* Get R/W sector size (WORD) */
	case GET_SECTOR_SIZE :
		*(WORD*)buff = fs_mmc->info.LogBlockSize;
		res = RES_OK;
		break;

	/* Get erase block size in unit of sector (DWORD) */
	case GET_BLOCK_SIZE :
		*(DWORD*)buff = fs_mmc->info.LogBlockSize / SD_DEFAULT_BLOCK_SIZE;
		res = RES_OK;
		break;

	default:
		res = RES_PARERR;
	}

	return res;
}
#endif

#endif // end with CONFIG_STM32_MMCSD_FATFS_SUPPORT

