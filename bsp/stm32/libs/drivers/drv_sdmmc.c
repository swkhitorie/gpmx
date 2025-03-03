#include "drv_sdmmc.h"

struct drv_sdmmc_t *drv_sdmmc_list[2] = {0, 0};

bool drv_sdmmc_pin_source_init(uint8_t num, uint8_t d0s, uint8_t d1s,
                uint8_t d2s, uint8_t d3s, uint8_t cmds, uint8_t clks)
{
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
		if (SDMMC2_PINCTRL_SOURCE(SDMMC_D0, d0s) != NULL && SDMMC2_PINCTRL_SOURCE(SDMMC_D1, d1s) != NULL
			&& SDMMC2_PINCTRL_SOURCE(SDMMC_D2, d2s) != NULL && SDMMC2_PINCTRL_SOURCE(SDMMC_D3, d3s) != NULL
			&& SDMMC2_PINCTRL_SOURCE(SDMMC_CMD, cmds) != NULL && SDMMC2_PINCTRL_SOURCE(SDMMC_CLK, clks) != NULL) {
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
        drv_gpio_init(d0_pin_node->port, d0_pin_node->pin, IOMODE_AFPP,
							GPIO_NOPULL, IO_SPEEDMAX, d0_pin_node->alternate, NULL);

        drv_gpio_init(d1_pin_node->port, d1_pin_node->pin, IOMODE_AFPP,
							GPIO_NOPULL, IO_SPEEDMAX, d1_pin_node->alternate, NULL);

        drv_gpio_init(d2_pin_node->port, d2_pin_node->pin, IOMODE_AFPP,
							GPIO_NOPULL, IO_SPEEDMAX, d2_pin_node->alternate, NULL);

        drv_gpio_init(d3_pin_node->port, d3_pin_node->pin, IOMODE_AFPP,
							GPIO_NOPULL, IO_SPEEDMAX, d3_pin_node->alternate, NULL);

        drv_gpio_init(cmd_pin_node->port, cmd_pin_node->pin, IOMODE_AFPP,
							GPIO_NOPULL, IO_SPEEDMAX, cmd_pin_node->alternate, NULL);

        drv_gpio_init(clk_pin_node->port, clk_pin_node->pin, IOMODE_AFPP,
							GPIO_NOPULL, IO_SPEEDMAX, clk_pin_node->alternate, NULL);
	} else {
		return false;
	}
	return true;
}

void drv_sdmmc_attr_init(struct drv_sdmmc_attr_t *obj, uint8_t num, uint8_t speed,
                    uint8_t d0s, uint8_t d1s, uint8_t d2s, uint8_t d3s,
                    uint8_t cmds, uint8_t clks, uint8_t priority)
{
    obj->num = num;
    obj->speed = speed * 1000 * 1000;//MHZ
    obj->d0s = d0s;
    obj->d1s = d1s;
    obj->d2s = d2s;
    obj->d3s = d3s;
    obj->cmds = cmds;
    obj->clks = clks;
    obj->priority = priority;
}
int drv_sdmmc_init(struct drv_sdmmc_t *obj, struct drv_sdmmc_attr_t *attr)
{
    int ret = 0;
    uint32_t clk_freq;
    RCC_OscInitTypeDef rcc_init;
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDMMC;
    PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

    obj->attr = *attr;
    SDMMC_TypeDef *sdmmc_array[2] = {SDMMC1, SDMMC2};
    uint32_t sdmmc_irq[2] = {SDMMC1_IRQn, SDMMC2_IRQn};
    obj->handle.Instance = sdmmc_array[obj->attr.num-1];

    switch (obj->attr.num) {
    case 1:
        __HAL_RCC_SDMMC1_CLK_ENABLE();
        break;
    case 2:
        __HAL_RCC_SDMMC2_CLK_ENABLE(); 
        break;
    }

    drv_sdmmc_pin_source_init(attr->num, attr->d0s, attr->d1s, attr->d2s, attr->d3s,
        attr->cmds, attr->clks);

    switch (obj->attr.num) {
    case 1:
        __HAL_RCC_SDMMC1_FORCE_RESET();
        __HAL_RCC_SDMMC1_RELEASE_RESET();
        break;
    case 2: 
        __HAL_RCC_SDMMC2_FORCE_RESET();
        __HAL_RCC_SDMMC2_RELEASE_RESET();
        break;
    }

    HAL_NVIC_SetPriority(sdmmc_irq[obj->attr.num-1], obj->attr.priority, 0);
    HAL_NVIC_EnableIRQ(sdmmc_irq[obj->attr.num-1]);

    // HAL_SD_DeInit(&obj->handle);
    // sdmmc clock = Periph clock / 2 * CLOCKDIV
    // printf("CLOCKDIV: %d \r\n", (200*1000*1000 / obj->attr.speed) / 2 );
    obj->handle.Init.ClockDiv            = 6; // 6 
    obj->handle.Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    obj->handle.Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
    obj->handle.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    obj->handle.Init.BusWide             = SDMMC_BUS_WIDE_4B;
    ret = obj->initret = HAL_SD_Init(&obj->handle);
    if (ret != HAL_OK)
        return 0x01;

    drv_sdmmc_list[obj->attr.num-1] = obj;
    return ret;
}

uint8_t drv_sdmmc_wait_ready(struct drv_sdmmc_t *obj)
{
	uint32_t loop = SD_TIMEOUT;

	/* Wait for the Erasing process is completed */
	/* Verify that SD card is ready to use after the Erase */
	while (loop > 0) {
		loop--;
		if (HAL_SD_GetCardState(&obj->handle) == HAL_SD_CARD_TRANSFER)
			return MSD_OK;
	}
	return MSD_ERROR;
}

void drv_sdmmc_getinfo(struct drv_sdmmc_t *obj)
{
    HAL_SD_GetCardInfo(&obj->handle, &obj->info);
}

uint8_t drv_sdmmc_getstate(struct drv_sdmmc_t *obj)
{
    return ((HAL_SD_GetCardState(&obj->handle) == HAL_SD_CARD_TRANSFER) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY);
}

uint8_t drv_sdmmc_erase(struct drv_sdmmc_t *obj, uint32_t start, uint32_t end)
{
    int ret;
    ret = (HAL_SD_Erase(&obj->handle, start, end) == HAL_OK) ? MSD_OK : MSD_ERROR;
    return ret;

}
uint8_t drv_sdmmc_read_blocks(struct drv_sdmmc_t *obj, 
    uint32_t *p, uint32_t addr, uint32_t num, enum __drv_rwway way)
{
    int ret;
    switch (way) {
    case RWPOLL:
        ret = (HAL_SD_ReadBlocks(&obj->handle, (uint8_t *)p, addr, num, 100U*num) == HAL_OK) ? MSD_OK : MSD_ERROR;
        break;
    case RWIT:
    case RWDMA:
        ret = (HAL_SD_ReadBlocks_DMA(&obj->handle, (uint8_t *)p, addr, num) == HAL_OK) ? MSD_OK : MSD_ERROR;
        break;
    }
    return ret;
}

uint8_t drv_sdmmc_write_blocks(struct drv_sdmmc_t *obj, 
    uint32_t *p, uint32_t addr, uint32_t num, enum __drv_rwway way)
{
    int ret;
    switch (way) {
    case RWPOLL:
        ret = (HAL_SD_WriteBlocks(&obj->handle, (uint8_t *)p, addr, num, 100U*num) == HAL_OK) ? MSD_OK : MSD_ERROR;
        break;
    case RWIT:
    case RWDMA:
        ret = (HAL_SD_WriteBlocks_DMA(&obj->handle, (uint8_t *)p, addr, num) == HAL_OK) ? MSD_OK : MSD_ERROR;
        break;
    }
    return ret;
}

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
    HAL_SD_IRQHandler(&drv_sdmmc_list[0]->handle);
}

void SDMMC2_IRQHandler(void)
{
    HAL_SD_IRQHandler(&drv_sdmmc_list[1]->handle);
}
