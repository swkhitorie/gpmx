#include <board_config.h>
#include <drv_uart.h>

#ifdef BSP_COM_PRINTF
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
FILE __stdin, __stdout, __stderr;
#endif

static void board_config_io();

struct drv_uart_t com1;
uint8_t com1_dma_rxbuff[256];
uint8_t com1_dma_txbuff[256];
uint8_t com1_txbuff[512];
uint8_t com1_rxbuff[512];

void board_bsp_init()
{
    board_config_io();

	VDD_5V_PERIPH_EN(true);
	VDD_5V_HIPOWER_EN(true);
    VDD_3V3_SENSORS_EN(true);

	BOARD_BLUE_LED(false);
	BOARD_RED_LED(false);

    struct drv_uart_attr_t com1_attr;
    struct drv_uart_dma_attr_t com1_rxdma_attr;
    struct drv_uart_dma_attr_t com1_txdma_attr;
    drv_uart_dma_attr_init(&com1_rxdma_attr, &com1_dma_rxbuff[0], 256, 2);
    drv_uart_dma_attr_init(&com1_txdma_attr, &com1_dma_txbuff[0], 256, 2);
    drv_uart_attr_init(&com1_attr, 115200, WL_8BIT, STB_1BIT, PARITY_NONE, 1);
    drv_uart_buff_init(&com1, &com1_txbuff[0], 512, &com1_rxbuff[0], 512);
    drv_uart_init(1, &com1, 2, 1, &com1_attr, &com1_txdma_attr, &com1_rxdma_attr);
    //drv_uart_init(1, &com1, 2, 1, &com1_attr, NULL, NULL);

#ifdef BSP_MODULE_USB_CHERRY
    HAL_Delay(600);
    cdc_acm_init(0, USB_OTG_FS_PERIPH_BASE);
#endif
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

void board_debug()
{
	printf("[v6c] usart test \r\n");
    board_blue_led_toggle();
}

#ifdef BSP_COM_PRINTF
int _write(int file, char *ptr, int len)
{
    const int stdin_fileno = 0;
    const int stdout_fileno = 1;
    const int stderr_fileno = 2;
    if (file == stdout_fileno) {
        drv_uart_send(&com1, ptr, len, RWDMA);
    }
    return len;
}
#endif

