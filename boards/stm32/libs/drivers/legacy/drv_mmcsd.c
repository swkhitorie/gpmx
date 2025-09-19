#include "drv_mmcsd.h"

#if defined (DRV_BSP_H7)
struct up_mmcsd_dev_s *mmcsd_list[2];

/****************************************************************************
 * Private Function
 ****************************************************************************/
static bool low_pinconfig(struct up_mmcsd_dev_s *dev)
{
    uint8_t num = dev->id;
    uint8_t d0s = dev->pin_d0;
    uint8_t d1s = dev->pin_d1;
    uint8_t d2s = dev->pin_d2;
    uint8_t d3s = dev->pin_d3;
    uint8_t cmds = dev->pin_cmd;
    uint8_t clks = dev->pin_clks;

	const struct pin_node *d0_pin_node;
	const struct pin_node *d1_pin_node;
	const struct pin_node *d2_pin_node;
	const struct pin_node *d3_pin_node;
	const struct pin_node *cmd_pin_node;
	const struct pin_node *clk_pin_node;
	uint32_t illegal;

	switch (num) {
	case 1:
		d0_pin_node = SDMMC1_PINCTRL_SOURCE(SDMMC_D0);
		d1_pin_node = SDMMC1_PINCTRL_SOURCE(SDMMC_D1);
		d2_pin_node = SDMMC1_PINCTRL_SOURCE(SDMMC_D2);
		d3_pin_node = SDMMC1_PINCTRL_SOURCE(SDMMC_D3);
		cmd_pin_node = SDMMC1_PINCTRL_SOURCE(SDMMC_CMD);
		clk_pin_node = SDMMC1_PINCTRL_SOURCE(SDMMC_CLK);
		illegal = d0_pin_node->port && d1_pin_node->port && 
							d2_pin_node->port && d3_pin_node->port && 
							cmd_pin_node->port && clk_pin_node->port;
		break;
	case 2:
		if (SDMMC2_PINCTRL_SOURCE(SDMMC_D0, d0s) != NULLPIN && 
            SDMMC2_PINCTRL_SOURCE(SDMMC_D1, d1s) != NULLPIN && 
            SDMMC2_PINCTRL_SOURCE(SDMMC_D2, d2s) != NULLPIN && 
            SDMMC2_PINCTRL_SOURCE(SDMMC_D3, d3s) != NULLPIN &&
            SDMMC2_PINCTRL_SOURCE(SDMMC_CMD, cmds) != NULLPIN &&
            SDMMC2_PINCTRL_SOURCE(SDMMC_CLK, clks) != NULLPIN) {
			d0_pin_node = SDMMC2_PINCTRL_SOURCE(SDMMC_D0, d0s);
			d1_pin_node = SDMMC2_PINCTRL_SOURCE(SDMMC_D1, d1s);
			d2_pin_node = SDMMC2_PINCTRL_SOURCE(SDMMC_D2, d2s);
			d3_pin_node = SDMMC2_PINCTRL_SOURCE(SDMMC_D3, d3s);
			cmd_pin_node = SDMMC2_PINCTRL_SOURCE(SDMMC_CMD, cmds);
			clk_pin_node = SDMMC2_PINCTRL_SOURCE(SDMMC_CLK, clks);
			illegal = d0_pin_node->port && d1_pin_node->port && 
                        d2_pin_node->port && d3_pin_node->port && 
                        cmd_pin_node->port && clk_pin_node->port;
		}else {
			return false;
		}
		break;
	default: return false;
	}

	if (illegal != 0) {
        // low_gpio_setup(d0_pin_node->port, d0_pin_node->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, d0_pin_node->alternate, NULL, NULL, 0);
        // low_gpio_setup(d1_pin_node->port, d1_pin_node->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, d1_pin_node->alternate, NULL, NULL, 0);
        // low_gpio_setup(d2_pin_node->port, d2_pin_node->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, d2_pin_node->alternate, NULL, NULL, 0);
        // low_gpio_setup(d3_pin_node->port, d3_pin_node->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, d3_pin_node->alternate, NULL, NULL, 0);
        // low_gpio_setup(cmd_pin_node->port, cmd_pin_node->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, cmd_pin_node->alternate, NULL, NULL, 0);
        // low_gpio_setup(clk_pin_node->port, clk_pin_node->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, clk_pin_node->alternate, NULL, NULL, 0);
	} else {
		return false;
	}
	return true;
}

int low_mmcsd_init(struct up_mmcsd_dev_s *dev)
{
    int ret = 0;
    uint32_t clk_freq;

    RCC_OscInitTypeDef rcc_init;
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDMMC;
    PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

    SDMMC_TypeDef *sdmmc_array[2] = {SDMMC1, SDMMC2};
    uint32_t sdmmc_irq[2] = {SDMMC1_IRQn, SDMMC2_IRQn};
    dev->handle.Instance = sdmmc_array[dev->id - 1];

    switch (dev->id) {
    case 1: __HAL_RCC_SDMMC1_CLK_ENABLE(); break;
    case 2: __HAL_RCC_SDMMC2_CLK_ENABLE();  break;
    }

    low_pinconfig(dev);

    switch (dev->id) {
    case 1: __HAL_RCC_SDMMC1_FORCE_RESET(); __HAL_RCC_SDMMC1_RELEASE_RESET(); break;
    case 2: __HAL_RCC_SDMMC2_FORCE_RESET(); __HAL_RCC_SDMMC2_RELEASE_RESET(); break;
    }

    HAL_NVIC_SetPriority(sdmmc_irq[dev->id - 1], dev->priority, 0);
    HAL_NVIC_EnableIRQ(sdmmc_irq[dev->id - 1]);

    // HAL_SD_DeInit(&obj->handle);
    // sdmmc clock = Periph clock / 2 * CLOCKDIV
    // printf("CLOCKDIV: %d \r\n", (200*1000*1000 / obj->attr.speed) / 2 );
    dev->handle.Init.ClockDiv            = 6; // 6 
    dev->handle.Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    dev->handle.Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
    dev->handle.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    dev->handle.Init.BusWide             = SDMMC_BUS_WIDE_4B;
    ret = dev->initret = HAL_SD_Init(&dev->handle);
    if (ret != HAL_OK)
        return 0x01;

    mmcsd_list[dev->id - 1] = dev;
    return ret;
}

uint8_t low_mmcsd_waitrdy(struct up_mmcsd_dev_s *dev)
{
	uint32_t loop = SD_TIMEOUT;

	/* Wait for the Erasing process is completed */
	/* Verify that SD card is ready to use after the Erase */
	while (loop > 0) {
		loop--;
		if (HAL_SD_GetCardState(&dev->handle) == HAL_SD_CARD_TRANSFER)
			return MSD_OK;
	}
	return MSD_ERROR;
}

void low_mmcsd_getinfo(struct up_mmcsd_dev_s *dev)
{
    HAL_SD_GetCardInfo(&dev->handle, &dev->info);
}

uint8_t low_mmcsd_getstat(struct up_mmcsd_dev_s *dev)
{
    return ((HAL_SD_GetCardState(&dev->handle) == HAL_SD_CARD_TRANSFER) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY);
}

uint8_t low_mmcsd_erase(struct up_mmcsd_dev_s *dev, uint32_t start, uint32_t end)
{
    int ret;
    ret = (HAL_SD_Erase(&dev->handle, start, end) == HAL_OK) ? MSD_OK : MSD_ERROR;
    return ret;

}
uint8_t low_mmcsd_reads(struct up_mmcsd_dev_s *dev, uint32_t *p, uint32_t addr, uint32_t num, enum __drv_rwway way)
{
    int ret;
    switch (way) {
    case RWPOLL:
        ret = (HAL_SD_ReadBlocks(&dev->handle, (uint8_t *)p, addr, num, 100U*num) == HAL_OK) ? MSD_OK : MSD_ERROR;
        break;
    case RWIT:
    case RWDMA:
        ret = (HAL_SD_ReadBlocks_DMA(&dev->handle, (uint8_t *)p, addr, num) == HAL_OK) ? MSD_OK : MSD_ERROR;
        break;
    }
    return ret;
}

uint8_t low_mmcsd_writes(struct up_mmcsd_dev_s *dev, uint32_t *p, uint32_t addr, uint32_t num, enum __drv_rwway way)
{
    int ret;
    switch (way) {
    case RWPOLL:
        ret = (HAL_SD_WriteBlocks(&dev->handle, (uint8_t *)p, addr, num, 100U*num) == HAL_OK) ? MSD_OK : MSD_ERROR;
        break;
    case RWIT:
    case RWDMA:
        ret = (HAL_SD_WriteBlocks_DMA(&dev->handle, (uint8_t *)p, addr, num) == HAL_OK) ? MSD_OK : MSD_ERROR;
        break;
    }
    return ret;
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

void SDMMC1_IRQHandler(void)
{
    HAL_SD_IRQHandler(&mmcsd_list[0]->handle);
}

void SDMMC2_IRQHandler(void)
{
    HAL_SD_IRQHandler(&mmcsd_list[1]->handle);
}

#endif
