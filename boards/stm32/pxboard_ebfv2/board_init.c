#include <board_config.h>

static SRAM_HandleTypeDef  sram_handle;
static FMC_NORSRAM_TimingTypeDef sram_timing;

void board_reboot()
{
    NVIC_SystemReset();
}

void board_get_uid(uint32_t *p)
{
    p[0] = *(volatile uint32_t*)(0x1FFF7A10);
    p[1] = *(volatile uint32_t*)(0x1FFF7A14);
    p[2] = *(volatile uint32_t*)(0x1FFF7A18);
}

uint32_t board_get_time()
{
    return HAL_GetTick();
}

void board_delay(uint32_t ms)
{
    HAL_Delay(ms);
}

uint32_t board_elapsed_time(const uint32_t timestamp)
{
    uint32_t now = HAL_GetTick();
    if (timestamp > now) {
        return 0;
    }
    return now - timestamp;
}

static void board_sram_init()
{
    GPIO_InitTypeDef gpio_init;

    sram_handle.Instance = FMC_NORSRAM_DEVICE;
    sram_handle.Extended = FMC_NORSRAM_EXTENDED_DEVICE;

    sram_timing.AddressSetupTime      = 0x00;
    sram_timing.AddressHoldTime       = 0x00;
    sram_timing.DataSetupTime         = 0x08;
    sram_timing.BusTurnAroundDuration = 0x00;
    sram_timing.CLKDivision           = 0x00;
    sram_timing.DataLatency           = 0x00;
    sram_timing.AccessMode            = FSMC_ACCESS_MODE_A;

    sram_handle.Init.NSBank = FSMC_NORSRAM_BANK4;
	sram_handle.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
	sram_handle.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
	sram_handle.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
	sram_handle.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
	sram_handle.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
	sram_handle.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
	sram_handle.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
	sram_handle.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
	sram_handle.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
	sram_handle.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
	sram_handle.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;           	  
	sram_handle.Init.ContinuousClock = FSMC_CONTINUOUS_CLOCK_SYNC_ASYNC;

    __HAL_RCC_FSMC_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    gpio_init.Mode      = GPIO_MODE_AF_PP;
    gpio_init.Pull      = GPIO_PULLUP;
    gpio_init.Speed     = GPIO_SPEED_HIGH;
    gpio_init.Alternate = GPIO_AF12_FSMC;

    gpio_init.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8     |\
                        GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 |\
                        GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOD, &gpio_init);

    gpio_init.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7     |\
                        GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |\
                        GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &gpio_init);

    gpio_init.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4     |\
                        GPIO_PIN_5 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOF, &gpio_init);

    gpio_init.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4     |\
                        GPIO_PIN_5 | GPIO_PIN_12;
    HAL_GPIO_Init(GPIOG, &gpio_init);
    HAL_SRAM_Init(&sram_handle, &sram_timing, &sram_timing);
}

static void board_config_power_rcc()
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = STM32_PLLCFG_PLL1M;
    RCC_OscInitStruct.PLL.PLLN = STM32_PLLCFG_PLL1N;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = STM32_PLLCFG_PLL1Q;

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

static void board_irq_reset()
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

void board_init()
{
    //SCB->VTOR = APP_LOAD_ADDRESS;

    //board_irq_reset();

    HAL_Init();

    board_config_power_rcc();

    board_sram_init();
}

void board_deinit()
{
    int i = 0;

    __set_PRIMASK(1); 

    HAL_RCC_DeInit();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    for (i = 0; i < 8; i++) {
        NVIC->ICER[i]=0xFFFFFFFF;
        NVIC->ICPR[i]=0xFFFFFFFF;
    }	

    __set_PRIMASK(0);
}
