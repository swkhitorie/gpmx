#include "board_config.h"

static void board_config_mmu();
static void board_config_cache();
static void board_config_power_rcc();
static void board_config_sdk();
static void board_config_io();

void board_config_sdk()
{
    HAL_Init();
}

void board_config_cache()
{
	SCB_InvalidateICache();
	SCB_InvalidateDCache();
    
    SCB_EnableICache();
	SCB_EnableDCache();
}

void board_config_mmu()
{
	MPU_Region_InitTypeDef MPU_InitStruct;

	HAL_MPU_Disable();
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress      = 0x24000000;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_512KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	// when isBufferable is MPU_ACCESS_BUFFERABLE, sd+fatfs can not read/write
	// when isBufferable is MPU_ACCESS_NOT_BUFFERABLE, sd+fatfs is available
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress      = 0x60000000;
	MPU_InitStruct.Size             = ARM_MPU_REGION_SIZE_64KB;	
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER1;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);
	
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress      = 0x38000000;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_64KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER2;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void board_config_power_rcc()
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Supply configuration update enable PWR_EXTERNAL_SOURCE_SUPPLY PWR_LDO_SUPPLY
    */
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

    /** Configure the main internal regulator output voltage
    */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);
    while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    * HSE: 16M -> 16M / PLLM * PLLN / PLLP
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI48;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
    RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

    /* HSE=16M Kernel Clock(480MHZ)= HSE/PLLM*PLLN/PLLP */
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 60;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    RCC_OscInitStruct.PLL.PLLR = 8;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    //RCC_OscInitStruct.PLL.PLLFRACN = 0;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        while(1) {}
    }

    /** Initializes the CPU, AHB and APB buses clocks */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                                |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
        while(1) {}
    }

    __HAL_RCC_CSI_ENABLE() ;
    __HAL_RCC_SYSCFG_CLK_ENABLE() ;

    HAL_EnableCompensationCell();
    __HAL_RCC_D2SRAM1_CLK_ENABLE();
    __HAL_RCC_D2SRAM2_CLK_ENABLE();
    __HAL_RCC_D2SRAM3_CLK_ENABLE();

    __HAL_RCC_BKPRAM_CLKAM_ENABLE();
    __HAL_RCC_D3SRAM1_CLKAM_ENABLE();

	for (int i = 0; i < 1000 * 10; i++);
}

void board_config_io()
{
	GPIO_InitTypeDef obj;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

	// Red Led
	obj.Pin = GPIO_nLED_RED_PIN;
	obj.Mode = GPIO_MODE_OUTPUT_PP;
	obj.Pull = GPIO_NOPULL;
	obj.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIO_nLED_RED_PORT, &obj);
	
	// Blue Led
	obj.Pin = GPIO_nLED_BLUE_PIN;
	obj.Mode = GPIO_MODE_OUTPUT_PP;
	obj.Pull = GPIO_NOPULL;
	obj.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIO_nLED_BLUE_PORT, &obj);

	// power in detector
	obj.Pin = GPIO_nPOWER_IN_A_PIN;
	obj.Mode = GPIO_MODE_INPUT;
	obj.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIO_nPOWER_IN_A_PORT, &obj);
	obj.Pin = GPIO_nPOWER_IN_B_PIN;
	obj.Mode = GPIO_MODE_INPUT;
	obj.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIO_nPOWER_IN_B_PORT, &obj);
	obj.Pin = GPIO_nPOWER_IN_C_PIN;
	obj.Mode = GPIO_MODE_INPUT;
	obj.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIO_nPOWER_IN_C_PORT, &obj);
	
	// periph output ctrl and end
	obj.Pin = GPIO_VDD_5V_PERIPH_nEN_PIN;
	obj.Mode = GPIO_MODE_OUTPUT_PP;
	obj.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIO_VDD_5V_PERIPH_nEN_PORT, &obj);
	obj.Pin = GPIO_VDD_5V_PERIPH_nOC_PIN;
	obj.Mode = GPIO_MODE_INPUT;
	obj.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIO_VDD_5V_PERIPH_nOC_PORT, &obj);
	
	// hipower output ctrl and end
	obj.Pin = GPIO_VDD_5V_HIPOWER_nEN_PIN;
	obj.Mode = GPIO_MODE_OUTPUT_PP;
	obj.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIO_VDD_5V_HIPOWER_nEN_PORT, &obj);
	obj.Pin = GPIO_VDD_5V_HIPOWER_nOC_PIN;
	obj.Mode = GPIO_MODE_INPUT;
	obj.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIO_VDD_5V_HIPOWER_nOC_PORT, &obj);

	obj.Pin = GPIO_VDD_3V3_SENSORS_EN_PIN;
	obj.Mode = GPIO_MODE_OUTPUT_PP;
	obj.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIO_VDD_3V3_SENSORS_EN_PORT, &obj);
	
	obj.Pin = GPIO_OTGFS_VBUS_PIN;
	obj.Mode = GPIO_MODE_INPUT;
	obj.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIO_OTGFS_VBUS_PORT, &obj);
}

void board_irqreset()
{
    /* clear RCC and all periphal interrupt */
    HAL_RCC_DeInit();
    for (int i = 0; i < 8; i++) {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    /* set SP->MSP */
    __set_CONTROL(0);
    /* enable all interrupt and exception */        
    __set_PRIMASK(0);
    __set_FAULTMASK(0);
    __set_BASEPRI(0);
    __enable_irq();
}

void board_reboot()
{
    NVIC_SystemReset();
}

void board_blue_led_toggle()
{
	int val = HAL_GPIO_ReadPin(GPIO_nLED_BLUE_PORT, GPIO_nLED_BLUE_PIN);
	HAL_GPIO_WritePin(GPIO_nLED_BLUE_PORT, GPIO_nLED_BLUE_PIN, !val);
}

void board_red_led_toggle()
{
	int val = HAL_GPIO_ReadPin(GPIO_nLED_RED_PORT, GPIO_nLED_RED_PIN);
	HAL_GPIO_WritePin(GPIO_nLED_RED_PORT, GPIO_nLED_RED_PIN, !val);
}

void board_init()
{
    SCB->VTOR = APP_LOAD_ADDRESS;
    board_irqreset();

    board_config_mmu();

    board_config_cache();

    board_config_sdk();

    board_config_power_rcc();

    board_config_io();

	VDD_5V_PERIPH_EN(true);
	VDD_5V_HIPOWER_EN(true);

	BOARD_BLUE_LED(false);
	BOARD_RED_LED(false);

    HAL_Delay(800);

	board_usb_init();
}
