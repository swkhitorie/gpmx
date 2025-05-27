#include "board_config.h"


static void board_config_power_rcc();

void board_irqreset()
{
    /* clear RCC and all periphal interrupt */
    // HAL_RCC_DeInit();
    for (int i = 0; i < 8; i++) {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    // /* set SP->MSP */
    // __set_CONTROL(0);
    // /* enable all interrupt and exception */        
    // __set_PRIMASK(0);
    // __set_FAULTMASK(0);
    // __set_BASEPRI(0);
    // __enable_irq();
}

void board_reboot()
{
    NVIC_SystemReset();
}

void board_config_power_rcc()
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = STM32_PLLCFG_PLL1M;
    RCC_OscInitStruct.PLL.PLLN = STM32_PLLCFG_PLL1N;
    RCC_OscInitStruct.PLL.PLLP = STM32_PLLCFG_PLL1P;
    RCC_OscInitStruct.PLL.PLLQ = STM32_PLLCFG_PLL1Q;   //I2S need 48MHz, but this config only 45MHz
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        while(1) {}
    }

    /** Initializes the CPU, AHB and APB buses clocks */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
        while(1) {}
    }

    __HAL_RCC_SYSCFG_CLK_ENABLE();
}

void board_init()
{
    SCB->VTOR = APP_LOAD_ADDRESS;
    // board_irqreset();

    board_config_power_rcc();

    HAL_Init();
}

