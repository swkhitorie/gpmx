#include "board_config.h"

static void board_config_io();
static void board_config_power_rcc();
static void board_config_sdk();

void board_irqreset()
{

}

void board_reboot()
{
    NVIC_SystemReset();
}

void board_config_io()
{
	GPIO_InitTypeDef obj;
    __HAL_RCC_GPIOE_CLK_ENABLE();
    obj.Pin = GPIO_nLED_PIN;
    obj.Mode = GPIO_MODE_OUTPUT_PP;
    obj.Pull = GPIO_NOPULL;
    obj.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIO_nLED_PORT, &obj);
}

void board_config_power_rcc()
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    /** Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 24;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;   //I2S need 48MHz, but this config only 45MHz
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        while(1) {}
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
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
    __HAL_RCC_PWR_CLK_ENABLE();

    HAL_Delay(1000);
}

void board_config_sdk()
{
    HAL_Init();
}

void board_init()
{
    SCB->VTOR = APP_LOAD_ADDRESS;
    board_irqreset();

    board_config_power_rcc();

    board_config_io();

    board_config_sdk();

    BOARD_LED(false);

    HAL_Delay(800);

    board_usb_init();
}

void board_led_toggle()
{
	int val = HAL_GPIO_ReadPin(GPIO_nLED_PORT, GPIO_nLED_PIN);
	HAL_GPIO_WritePin(GPIO_nLED_PORT, GPIO_nLED_PIN, !val);
}
