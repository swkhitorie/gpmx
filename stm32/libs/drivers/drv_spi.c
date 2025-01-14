#include "drv_spi.h"

static bool drv_spi_pin_source_init(uint8_t num, 
    uint8_t nss_selec, uint8_t sck_selec, 
	uint8_t miso_selec, uint8_t mosi_selec)
{
	const struct pin_node *nss_node;
	const struct pin_node *sck_node;
	const struct pin_node *miso_node;
	const struct pin_node *mosi_node;
	uint32_t illegal;
	
	switch (num) {
	case 1:
		if (PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(1, SPI_PIN_NSS, nss_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(1, SPI_PIN_SCK, sck_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(1, SPI_PIN_MISO, miso_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(1, SPI_PIN_MOSI, mosi_selec)) != NULL) {
			nss_node = SPI_PINCTRL_SOURCE(1, SPI_PIN_NSS, nss_selec);
			sck_node = SPI_PINCTRL_SOURCE(1, SPI_PIN_SCK, sck_selec);
			miso_node = SPI_PINCTRL_SOURCE(1, SPI_PIN_MISO, miso_selec);
			mosi_node = SPI_PINCTRL_SOURCE(1, SPI_PIN_MOSI, mosi_selec);
			illegal = nss_node->port && nss_node->port
						&& nss_node->port && nss_node->port;
		}else {
			return false;
		}
		break;
	case 2:
		if (PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(2, SPI_PIN_NSS, nss_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(2, SPI_PIN_SCK, sck_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(2, SPI_PIN_MISO, miso_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(2, SPI_PIN_MOSI, mosi_selec)) != NULL) {
			nss_node = SPI_PINCTRL_SOURCE(2, SPI_PIN_NSS, nss_selec);
			sck_node = SPI_PINCTRL_SOURCE(2, SPI_PIN_SCK, sck_selec);
			miso_node = SPI_PINCTRL_SOURCE(2, SPI_PIN_MISO, miso_selec);
			mosi_node = SPI_PINCTRL_SOURCE(2, SPI_PIN_MOSI, mosi_selec);
			illegal = nss_node->port && nss_node->port
						&& nss_node->port && nss_node->port;
		}else {
			return false;
		}
		break;
	case 3:
		if (PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(3, SPI_PIN_NSS, nss_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(3, SPI_PIN_SCK, sck_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(3, SPI_PIN_MISO, miso_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(3, SPI_PIN_MOSI, mosi_selec)) != NULL) {
			nss_node = SPI_PINCTRL_SOURCE(3, SPI_PIN_NSS, nss_selec);
			sck_node = SPI_PINCTRL_SOURCE(3, SPI_PIN_SCK, sck_selec);
			miso_node = SPI_PINCTRL_SOURCE(3, SPI_PIN_MISO, miso_selec);
			mosi_node = SPI_PINCTRL_SOURCE(3, SPI_PIN_MOSI, mosi_selec);
			illegal = nss_node->port && nss_node->port
						&& nss_node->port && nss_node->port;
		}else {
			return false;
		}
		break;
	case 4:
		if (PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(4, SPI_PIN_NSS, nss_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(4, SPI_PIN_SCK, sck_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(4, SPI_PIN_MISO, miso_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(4, SPI_PIN_MOSI, mosi_selec)) != NULL) {
			nss_node = SPI_PINCTRL_SOURCE(4, SPI_PIN_NSS, nss_selec);
			sck_node = SPI_PINCTRL_SOURCE(4, SPI_PIN_SCK, sck_selec);
			miso_node = SPI_PINCTRL_SOURCE(4, SPI_PIN_MISO, miso_selec);
			mosi_node = SPI_PINCTRL_SOURCE(4, SPI_PIN_MOSI, mosi_selec);
			illegal = nss_node->port && nss_node->port
						&& nss_node->port && nss_node->port;
		}else {
			return false;
		}
		break;
	case 5:
		if (PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(5, SPI_PIN_NSS, nss_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(5, SPI_PIN_SCK, sck_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(5, SPI_PIN_MISO, miso_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(5, SPI_PIN_MOSI, mosi_selec)) != NULL) {
			nss_node = SPI_PINCTRL_SOURCE(5, SPI_PIN_NSS, nss_selec);
			sck_node = SPI_PINCTRL_SOURCE(5, SPI_PIN_SCK, sck_selec);
			miso_node = SPI_PINCTRL_SOURCE(5, SPI_PIN_MISO, miso_selec);
			mosi_node = SPI_PINCTRL_SOURCE(5, SPI_PIN_MOSI, mosi_selec);
			illegal = nss_node->port && nss_node->port
						&& nss_node->port && nss_node->port;
		}else {
			return false;
		}
		break;
	case 6:
		if (PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(6, SPI_PIN_NSS, nss_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(6, SPI_PIN_SCK, sck_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(6, SPI_PIN_MISO, miso_selec)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(6, SPI_PIN_MOSI, mosi_selec)) != NULL) {
			nss_node = SPI_PINCTRL_SOURCE(6, SPI_PIN_NSS, nss_selec);
			sck_node = SPI_PINCTRL_SOURCE(6, SPI_PIN_SCK, sck_selec);
			miso_node = SPI_PINCTRL_SOURCE(6, SPI_PIN_MISO, miso_selec);
			mosi_node = SPI_PINCTRL_SOURCE(6, SPI_PIN_MOSI, mosi_selec);
			illegal = nss_node->port && nss_node->port
						&& nss_node->port && nss_node->port;
		}else {
			return false;
		}
		break;
	default: return false;
	}
	
	if (illegal != 0) {
        // drv_gpio_init(nss_node->port, nss_node->pin_num, IOMODE_AFPP,
		// 					IO_PULLUP, IO_SPEEDMAX, nss_node->alternate, NULL);
        drv_gpio_init(sck_node->port, sck_node->pin_num, IOMODE_AFPP,
							IO_PULLUP, IO_SPEEDMAX, sck_node->alternate, NULL);
        drv_gpio_init(miso_node->port, miso_node->pin_num, IOMODE_AFPP,
							IO_PULLUP, IO_SPEEDMAX, miso_node->alternate, NULL);
        drv_gpio_init(mosi_node->port, mosi_node->pin_num, IOMODE_AFPP,
							IO_PULLUP, IO_SPEEDMAX, mosi_node->alternate, NULL);
	}else {
		return false;
	}
	return true;
}

void drv_spi_attr_init(struct drv_spi_attr_t *obj, uint8_t mode)
{
    obj->mode = mode;
}

void drv_spi_init(uint8_t num, uint32_t prescaler, struct drv_spi_t *obj,
    uint8_t nss, uint8_t sck, uint8_t miso, uint8_t mosi)
{
    uint32_t spiCLK[6] = {RCC_PERIPHCLK_SPI1, RCC_PERIPHCLK_SPI2, RCC_PERIPHCLK_SPI3, RCC_PERIPHCLK_SPI4, RCC_PERIPHCLK_SPI5, RCC_PERIPHCLK_SPI6};
	uint32_t clsSrc[6] = {RCC_SPI1CLKSOURCE_PLL, RCC_SPI2CLKSOURCE_PLL, RCC_SPI3CLKSOURCE_PLL, RCC_SPI4CLKSOURCE_D2PCLK1, RCC_SPI5CLKSOURCE_D2PCLK1, RCC_SPI6CLKSOURCE_D3PCLK1};
	SPI_TypeDef *SPIs[6] = {SPI1, SPI2, SPI3, SPI4, SPI5, SPI6};

	RCC_PeriphCLKInitTypeDef SPIxClkInit = {0};
	SPIxClkInit.PeriphClockSelection = spiCLK[num-1];
	if (num <= 3)       SPIxClkInit.Spi123ClockSelection = clsSrc[num-1];
	else if (num <= 5)  SPIxClkInit.Spi45ClockSelection = clsSrc[num-1];
	else                SPIxClkInit.Spi6ClockSelection = clsSrc[num-1];
	HAL_RCCEx_PeriphCLKConfig(&SPIxClkInit);

	switch (num) {
	case 1:	__HAL_RCC_SPI1_CLK_ENABLE();	break;
	case 2:	__HAL_RCC_SPI2_CLK_ENABLE();	break;
	case 3:	__HAL_RCC_SPI3_CLK_ENABLE();	break;
	case 4:	__HAL_RCC_SPI4_CLK_ENABLE();	break;
	case 5:	__HAL_RCC_SPI5_CLK_ENABLE();	break;
	case 6:	__HAL_RCC_SPI6_CLK_ENABLE();	break;
	}

	drv_spi_pin_source_init(num, nss, sck, miso, mosi);	

	obj->hspi.Instance               = SPIs[num - 1];
    obj->hspi.Init.BaudRatePrescaler = prescaler;
	obj->hspi.Init.Mode              = SPI_MODE_MASTER;
	obj->hspi.Init.Direction         = SPI_DIRECTION_2LINES;
	obj->hspi.Init.DataSize          = SPI_DATASIZE_8BIT;
	switch (mode) {
	case SPI_MODE0:
		obj->hspi.Init.CLKPolarity       = SPI_POLARITY_LOW;
		obj->hspi.Init.CLKPhase          = SPI_PHASE_1EDGE;
		break;
	case SPI_MODE1:
		obj->hspi.Init.CLKPolarity       = SPI_POLARITY_LOW;
		obj->hspi.Init.CLKPhase          = SPI_PHASE_2EDGE;
		break;
	case SPI_MODE2:
		obj->hspi.Init.CLKPolarity       = SPI_POLARITY_HIGH;
		obj->hspi.Init.CLKPhase          = SPI_PHASE_1EDGE;
		break;
	case SPI_MODE3:
		obj->hspi.Init.CLKPolarity       = SPI_POLARITY_HIGH;
		obj->hspi.Init.CLKPhase          = SPI_PHASE_2EDGE;
		break;	
	default :break;
	}
	obj->hspi.Init.NSS               = SPI_NSS_SOFT;
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

void drv_spi_write(struct drv_spi_t *obj, const uint8_t *p, uint16_t len, enum __drv_rwway way)
{
	HAL_SPI_Transmit(&obj->hspi, p, len, 5000);
}

void drv_spi_read(struct drv_spi_t *obj, uint8_t *p, uint16_t len)
{
	HAL_SPI_Receive(&obj->hspi, p, len, 5000);
}

int drv_spi_readwrite(struct drv_spi_t *obj, uint8_t *ptx, uint8_t *prx, uint16_t size)
{
    int8_t res = HAL_SPI_TransmitReceive(&obj->hspi, ptx, prx, size, 5000);
	return res;  
}
