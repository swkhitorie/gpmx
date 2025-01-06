/**
 * low level driver for stm32h7 series, base on cubehal library
 * module spi
*/

#include "include/lld_spi.h"

static bool lowlevel_spi_pin_source_init(lld_spi_t *obj, uint8_t spi_num, 
    uint8_t spi_nss_selec, uint8_t spi_sck_selec, 
	uint8_t spi_miso_selec, uint8_t spi_mosi_selec)
{
	const struct pin_node *spi_nss_node;
	const struct pin_node *spi_sck_node;
	const struct pin_node *spi_miso_node;
	const struct pin_node *spi_mosi_node;
	uint32_t is_pin_node_initable;
	
	switch (spi_num) {
	case 1:
		if (PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(1, SPI_PIN_NSS, spi_nss_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(1, SPI_PIN_SCK, spi_sck_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(1, SPI_PIN_MISO, spi_miso_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(1, SPI_PIN_MOSI, spi_mosi_selec)) != NULL) {
			spi_nss_node = SPI_PINCTRL_SOURCE(1, SPI_PIN_NSS, spi_nss_selec);
			spi_sck_node = SPI_PINCTRL_SOURCE(1, SPI_PIN_SCK, spi_sck_selec);
			spi_miso_node = SPI_PINCTRL_SOURCE(1, SPI_PIN_MISO, spi_miso_selec);
			spi_mosi_node = SPI_PINCTRL_SOURCE(1, SPI_PIN_MOSI, spi_mosi_selec);
			is_pin_node_initable = spi_nss_node->gpio_port && spi_nss_node->gpio_port
						&& spi_nss_node->gpio_port && spi_nss_node->gpio_port;
		}else {
			return false;
		}
		break;
	case 2:
		if (PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(2, SPI_PIN_NSS, spi_nss_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(2, SPI_PIN_SCK, spi_sck_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(2, SPI_PIN_MISO, spi_miso_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(2, SPI_PIN_MOSI, spi_mosi_selec)) != NULL) {
			spi_nss_node = SPI_PINCTRL_SOURCE(2, SPI_PIN_NSS, spi_nss_selec);
			spi_sck_node = SPI_PINCTRL_SOURCE(2, SPI_PIN_SCK, spi_sck_selec);
			spi_miso_node = SPI_PINCTRL_SOURCE(2, SPI_PIN_MISO, spi_miso_selec);
			spi_mosi_node = SPI_PINCTRL_SOURCE(2, SPI_PIN_MOSI, spi_mosi_selec);
			is_pin_node_initable = spi_nss_node->gpio_port && spi_nss_node->gpio_port
						&& spi_nss_node->gpio_port && spi_nss_node->gpio_port;
		}else {
			return false;
		}
		break;
	case 3:
		if (PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(3, SPI_PIN_NSS, spi_nss_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(3, SPI_PIN_SCK, spi_sck_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(3, SPI_PIN_MISO, spi_miso_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(3, SPI_PIN_MOSI, spi_mosi_selec)) != NULL) {
			spi_nss_node = SPI_PINCTRL_SOURCE(3, SPI_PIN_NSS, spi_nss_selec);
			spi_sck_node = SPI_PINCTRL_SOURCE(3, SPI_PIN_SCK, spi_sck_selec);
			spi_miso_node = SPI_PINCTRL_SOURCE(3, SPI_PIN_MISO, spi_miso_selec);
			spi_mosi_node = SPI_PINCTRL_SOURCE(3, SPI_PIN_MOSI, spi_mosi_selec);
			is_pin_node_initable = spi_nss_node->gpio_port && spi_nss_node->gpio_port
						&& spi_nss_node->gpio_port && spi_nss_node->gpio_port;
		}else {
			return false;
		}
		break;
	case 4:
		if (PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(4, SPI_PIN_NSS, spi_nss_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(4, SPI_PIN_SCK, spi_sck_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(4, SPI_PIN_MISO, spi_miso_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(4, SPI_PIN_MOSI, spi_mosi_selec)) != NULL) {
			spi_nss_node = SPI_PINCTRL_SOURCE(4, SPI_PIN_NSS, spi_nss_selec);
			spi_sck_node = SPI_PINCTRL_SOURCE(4, SPI_PIN_SCK, spi_sck_selec);
			spi_miso_node = SPI_PINCTRL_SOURCE(4, SPI_PIN_MISO, spi_miso_selec);
			spi_mosi_node = SPI_PINCTRL_SOURCE(4, SPI_PIN_MOSI, spi_mosi_selec);
			is_pin_node_initable = spi_nss_node->gpio_port && spi_nss_node->gpio_port
						&& spi_nss_node->gpio_port && spi_nss_node->gpio_port;
		}else {
			return false;
		}
		break;
	case 5:
		if (PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(5, SPI_PIN_NSS, spi_nss_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(5, SPI_PIN_SCK, spi_sck_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(5, SPI_PIN_MISO, spi_miso_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(5, SPI_PIN_MOSI, spi_mosi_selec)) != NULL) {
			spi_nss_node = SPI_PINCTRL_SOURCE(5, SPI_PIN_NSS, spi_nss_selec);
			spi_sck_node = SPI_PINCTRL_SOURCE(5, SPI_PIN_SCK, spi_sck_selec);
			spi_miso_node = SPI_PINCTRL_SOURCE(5, SPI_PIN_MISO, spi_miso_selec);
			spi_mosi_node = SPI_PINCTRL_SOURCE(5, SPI_PIN_MOSI, spi_mosi_selec);
			is_pin_node_initable = spi_nss_node->gpio_port && spi_nss_node->gpio_port
						&& spi_nss_node->gpio_port && spi_nss_node->gpio_port;
		}else {
			return false;
		}
		break;
	case 6:
		if (PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(6, SPI_PIN_NSS, spi_nss_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(6, SPI_PIN_SCK, spi_sck_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(6, SPI_PIN_MISO, spi_miso_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(6, SPI_PIN_MOSI, spi_mosi_selec)) != NULL) {
			spi_nss_node = SPI_PINCTRL_SOURCE(6, SPI_PIN_NSS, spi_nss_selec);
			spi_sck_node = SPI_PINCTRL_SOURCE(6, SPI_PIN_SCK, spi_sck_selec);
			spi_miso_node = SPI_PINCTRL_SOURCE(6, SPI_PIN_MISO, spi_miso_selec);
			spi_mosi_node = SPI_PINCTRL_SOURCE(6, SPI_PIN_MOSI, spi_mosi_selec);
			is_pin_node_initable = spi_nss_node->gpio_port && spi_nss_node->gpio_port
						&& spi_nss_node->gpio_port && spi_nss_node->gpio_port;
		}else {
			return false;
		}
		break;
	default: return false;
	}
	
	if (is_pin_node_initable != 0) {
        lld_gpio_init(&obj->cs_pin, spi_nss_node->gpio_port, spi_nss_node->pin_num, GPIO_MODE_AF_PP,
							GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH, spi_nss_node->alternate);
        lld_gpio_init(&obj->sck_pin, spi_sck_node->gpio_port, spi_sck_node->pin_num, GPIO_MODE_AF_PP,
							GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH, spi_sck_node->alternate);
        lld_gpio_init(&obj->miso_pin, spi_miso_node->gpio_port, spi_miso_node->pin_num, GPIO_MODE_AF_PP,
							GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH, spi_miso_node->alternate);
        lld_gpio_init(&obj->mosi_pin, spi_mosi_node->gpio_port, spi_mosi_node->pin_num, GPIO_MODE_AF_PP,
							GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH, spi_mosi_node->alternate);
        lld_gpio_set(&obj->sck_pin, true);
        lld_gpio_set(&obj->miso_pin, true);
        lld_gpio_set(&obj->mosi_pin, true);
	}else {
		return false;
	}
	return true;
}

void lld_spi_init(lld_spi_t *obj, uint8_t num, uint32_t prescaler, lld_spi_mode mode,
    uint8_t spi_nss_selec, uint8_t spi_sck_selec, uint8_t spi_miso_selec, uint8_t spi_mosi_selec)
{
    obj->spinum = num;
    obj->prescaler = prescaler;
    obj->mode = mode;
    
	switch (num) {
	case 1:	__HAL_RCC_SPI1_CLK_ENABLE();	break;
	case 2:	__HAL_RCC_SPI2_CLK_ENABLE();	break;
	case 3:	__HAL_RCC_SPI3_CLK_ENABLE();	break;
	case 4:	__HAL_RCC_SPI4_CLK_ENABLE();	break;
	case 5:	__HAL_RCC_SPI5_CLK_ENABLE();	break;
	case 6:	__HAL_RCC_SPI6_CLK_ENABLE();	break;
	}
    uint32_t spiCLK[6] = {RCC_PERIPHCLK_SPI1, RCC_PERIPHCLK_SPI2, RCC_PERIPHCLK_SPI3, RCC_PERIPHCLK_SPI4, RCC_PERIPHCLK_SPI5, RCC_PERIPHCLK_SPI6};
	uint32_t clsSrc[6] = {RCC_SPI1CLKSOURCE_PLL, RCC_SPI2CLKSOURCE_PLL, RCC_SPI3CLKSOURCE_PLL, RCC_SPI4CLKSOURCE_D2PCLK1, RCC_SPI5CLKSOURCE_D2PCLK1, RCC_SPI6CLKSOURCE_D3PCLK1};
	SPI_TypeDef *SPIs[6] = {SPI1, SPI2, SPI3, SPI4, SPI5, SPI6};
    
	RCC_PeriphCLKInitTypeDef SPIxClkInit = {0};
	SPIxClkInit.PeriphClockSelection = spiCLK[num - 1];
	if (num <= 3)       SPIxClkInit.Spi123ClockSelection = clsSrc[num - 1];
	else if (num <= 5)  SPIxClkInit.Spi45ClockSelection = clsSrc[num - 1];
	else                SPIxClkInit.Spi6ClockSelection = clsSrc[num - 1];
	HAL_RCCEx_PeriphCLKConfig(&SPIxClkInit);

	lowlevel_spi_pin_source_init(obj, num, spi_nss_selec, spi_sck_selec, spi_miso_selec, spi_mosi_selec);	

	obj->hspi.Instance               = SPIs[num - 1];
    obj->hspi.Init.BaudRatePrescaler = prescaler;
	obj->hspi.Init.Mode              = SPI_MODE_MASTER;
	obj->hspi.Init.Direction         = SPI_DIRECTION_2LINES;
	obj->hspi.Init.DataSize          = SPI_DATASIZE_8BIT;

	switch (mode) {
	case SPI_MODE_0:
		obj->hspi.Init.CLKPolarity       = SPI_POLARITY_LOW;
		obj->hspi.Init.CLKPhase          = SPI_PHASE_1EDGE;
		break;
	case SPI_MODE_1:
		obj->hspi.Init.CLKPolarity       = SPI_POLARITY_LOW;
		obj->hspi.Init.CLKPhase          = SPI_PHASE_2EDGE;
		break;
	case SPI_MODE_2:
		obj->hspi.Init.CLKPolarity       = SPI_POLARITY_HIGH;
		obj->hspi.Init.CLKPhase          = SPI_PHASE_1EDGE;
		break;
	case SPI_MODE_3:
		obj->hspi.Init.CLKPolarity       = SPI_POLARITY_HIGH;
		obj->hspi.Init.CLKPhase          = SPI_PHASE_2EDGE;
		break;	
	default :break;
	}

	obj->hspi.Init.NSS             	= SPI_NSS_SOFT;
	obj->hspi.Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;
	obj->hspi.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE; // SPI_MASTER_KEEP_IO_STATE_DISABLE SPI_MASTER_KEEP_IO_STATE_ENABLE
	obj->hspi.Init.FirstBit          = SPI_FIRSTBIT_MSB;
	obj->hspi.Init.TIMode            = SPI_TIMODE_DISABLE;
	obj->hspi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    obj->hspi.Init.CRCPolynomial     = 7;
    obj->hspi.Init.CRCPolynomial     = 8;
    obj->hspi.Init.FifoThreshold     = SPI_FIFO_THRESHOLD_01DATA;

    __HAL_SPI_DISABLE(&obj->hspi);	
	HAL_SPI_DeInit(&obj->hspi);
	HAL_SPI_Init(&obj->hspi);
	__HAL_SPI_ENABLE(&obj->hspi);
}

void lld_spi_write(lld_spi_t *obj, const uint8_t *p, uint16_t len, lld_rwway way)
{
    HAL_SPI_Transmit(&obj->hspi, p, len, 5000);
}

void lld_spi_read(lld_spi_t *obj, uint8_t *p, uint16_t len)
{
    HAL_SPI_Receive(&obj->hspi, p, len, 5000);
}

int lld_spi_readwrite(lld_spi_t *obj, uint8_t *ptx, uint8_t *prx, uint16_t size)
{
    int8_t res = HAL_SPI_TransmitReceive(&obj->hspi, ptx, prx, size, 5000);
	return res;  
}
