#include "board_config.h"

static void board_config_mmu();
static void board_config_cache();
static void board_config_power_rcc();

void board_config_cache()
{
	SCB_InvalidateICache();
	SCB_InvalidateDCache();
    
    SCB_EnableICache();
	SCB_EnableDCache();
}

void board_config_mmu()
{
    MPU_Region_InitTypeDef MPU_InitStruct = {0};
    HAL_MPU_Disable();

    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER0;
    MPU_InitStruct.BaseAddress = 0x0;
    MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
    MPU_InitStruct.SubRegionDisable = 0x87;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  //  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  //  MPU_InitStruct.BaseAddress      = 0x24000000;
  //  MPU_InitStruct.Size             = MPU_REGION_SIZE_512KB;
  //  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  //  MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
  //  MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
  //  MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  //  MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
  //  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
  //  MPU_InitStruct.SubRegionDisable = 0x00;
  //  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void board_config_power_rcc()
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /*AXI clock gating */
    RCC->CKGAENR = 0xFFFFFFFF;

    /** Supply configuration update enable */
    HAL_PWREx_ConfigSupply(PWR_SMPS_1V8_SUPPLIES_LDO);

    /** Configure the main internal regulator output voltage */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);
    while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

    /** Initializes the RCC */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = STM32_PLLCFG_PLL1M;
    RCC_OscInitStruct.PLL.PLLN = STM32_PLLCFG_PLL1N;
    RCC_OscInitStruct.PLL.PLLP = STM32_PLLCFG_PLL1P;
    RCC_OscInitStruct.PLL.PLLQ = STM32_PLLCFG_PLL1Q;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        while (1) {}
    }

    /** Initializes the CPU, AHB and APB buses clocks */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                                |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK) {
        while (1) {}
    }

    __HAL_RCC_CSI_ENABLE() ;
    __HAL_RCC_SYSCFG_CLK_ENABLE() ;

    HAL_EnableCompensationCell();

    __HAL_RCC_BKPRAM_CLKAM_ENABLE();

	for (int i = 0; i < 3000; i++);
}

void board_irq_reset()
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

void board_init()
{
    // SCB->VTOR = APP_LOAD_ADDRESS;

    // board_irq_reset();

    board_config_mmu();

    board_config_cache();

    HAL_Init();

    board_config_power_rcc();

    board_bsp_init();
}
