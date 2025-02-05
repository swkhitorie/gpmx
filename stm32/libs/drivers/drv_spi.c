#include "drv_spi.h"

struct drv_spi_t *drv_spi_list[DRV_SPI_PERIPHAL_NUM];

static bool drv_spi_pin_source_init(uint8_t num, 
    uint8_t nss_selec, uint8_t sck_selec, 
	uint8_t miso_selec, uint8_t mosi_selec)
{
#if defined (DRV_BSP_H7)
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
        drv_gpio_init(sck_node->port, sck_node->pin, IOMODE_AFPP,
							IO_PULLUP, IO_SPEEDMAX, sck_node->alternate, NULL);
        drv_gpio_init(miso_node->port, miso_node->pin, IOMODE_AFPP,
							IO_PULLUP, IO_SPEEDMAX, miso_node->alternate, NULL);
        drv_gpio_init(mosi_node->port, mosi_node->pin, IOMODE_AFPP,
							IO_PULLUP, IO_SPEEDMAX, mosi_node->alternate, NULL);
	}else {
		return false;
	}
	return true;
#endif

#if defined (DRV_BSP_F1)
    if (nss_selec == 0) {
        GPIO_TypeDef *spi_port[3] =  { GPIOA,		GPIOB,      GPIOB };
        uint16_t       sck_pin[3] =  {   5,         13,          3    };
        uint16_t       miso_pin[3] = {   6,         14,          4    };
		uint16_t       mosi_pin[3] = {   7,         15,          5    };

        drv_gpio_init(spi_port[num-1], sck_pin[num-1], IOMODE_AFPP,   IO_NOPULL, IO_SPEEDHIGH, NULL);
        drv_gpio_init(spi_port[num-1], mosi_pin[num-1], IOMODE_AFPP,  IO_NOPULL, IO_SPEEDHIGH, NULL);
        drv_gpio_init(spi_port[num-1], miso_pin[num-1], IOMODE_INPUT, IO_NOPULL, IO_SPEEDHIGH, NULL);
    } else if (nss_selec == 1 && num == 1) {
        drv_gpio_init(GPIOB, 3, IOMODE_AFPP,   IO_NOPULL, IO_SPEEDHIGH, NULL);  //sck
        drv_gpio_init(GPIOB, 4, IOMODE_AFPP,  IO_NOPULL, IO_SPEEDHIGH, NULL);   //miso
        drv_gpio_init(GPIOB, 5, IOMODE_INPUT, IO_NOPULL, IO_SPEEDHIGH, NULL);   //mosi
    }
	return true;
#endif

#if defined (DRV_BSP_F4)
    if (nss_selec == 0) {
        GPIO_TypeDef *spi_port[3] =  { GPIOA,		GPIOB,      GPIOB };
        uint16_t       sck_pin[3] =  {   5,         13,          3    };
        uint16_t       miso_pin[3] = {   6,         14,          4    };
		uint16_t       mosi_pin[3] = {   7,         15,          5    };
        uint32_t       alternate[3] = {GPIO_AF5_SPI1, GPIO_AF5_SPI2, GPIO_AF6_SPI3};

        drv_gpio_init(spi_port[num-1], sck_pin[num-1], IOMODE_AFPP,   IO_NOPULL, IO_SPEEDHIGH, alternate[num-1], NULL);
        drv_gpio_init(spi_port[num-1], mosi_pin[num-1], IOMODE_AFPP,  IO_NOPULL, IO_SPEEDHIGH, alternate[num-1], NULL);
        drv_gpio_init(spi_port[num-1], miso_pin[num-1], IOMODE_INPUT, IO_NOPULL, IO_SPEEDHIGH, alternate[num-1], NULL);
    } else if (nss_selec == 1) {
        GPIO_TypeDef *spi_port[3] =  { GPIOB,		GPIOB,      GPIOC };
        uint16_t       sck_pin[3] =  {   3,         10,          10   };
        uint16_t       miso_pin[3] = {   4,         2,          11    };
		uint16_t       mosi_pin[3] = {   5,         3,          12    };
        uint32_t       alternate[3] = {GPIO_AF5_SPI1, GPIO_AF5_SPI2, GPIO_AF6_SPI3};

        drv_gpio_init(spi_port[num-1], sck_pin[num-1], IOMODE_AFPP,   IO_NOPULL, IO_SPEEDHIGH, alternate[num-1], NULL);
		if (num == 2) {
			spi_port[num-1] = GPIOC;
		}
        drv_gpio_init(spi_port[num-1], mosi_pin[num-1], IOMODE_AFPP,  IO_NOPULL, IO_SPEEDHIGH, alternate[num-1], NULL);
        drv_gpio_init(spi_port[num-1], miso_pin[num-1], IOMODE_INPUT, IO_NOPULL, IO_SPEEDHIGH, alternate[num-1], NULL);
    }
	return true;
#endif
}

void drv_spi_attr_init(struct drv_spi_attr_t *obj, uint32_t mode, uint32_t databits, uint32_t speed)
{
    obj->mode = mode;
	obj->databits = databits;
	obj->speed = speed;
}

void drv_spi_init(uint8_t num, struct drv_spi_t *obj, struct drv_spi_attr_t *attr,
    uint8_t nss, uint8_t sck, uint8_t miso, uint8_t mosi)
{
#if defined (DRV_BSP_H7)
    uint32_t spi_clk[6] = {
		RCC_PERIPHCLK_SPI1, RCC_PERIPHCLK_SPI2, RCC_PERIPHCLK_SPI3,
		RCC_PERIPHCLK_SPI4, RCC_PERIPHCLK_SPI5, RCC_PERIPHCLK_SPI6
	};
	uint32_t spi_clk_src[6] = {
		RCC_SPI1CLKSOURCE_PLL, RCC_SPI2CLKSOURCE_PLL, RCC_SPI3CLKSOURCE_PLL,
		RCC_SPI4CLKSOURCE_D2PCLK1, RCC_SPI5CLKSOURCE_D2PCLK1, RCC_SPI6CLKSOURCE_D3PCLK1
	};
	RCC_PeriphCLKInitTypeDef spi_clk_init_obj = {0};
	spi_clk_init_obj.PeriphClockSelection = spi_clk[num-1];
	if (num <= 3)      spi_clk_init_obj.Spi123ClockSelection = spi_clk_src[num-1];
	else if (num <= 5) spi_clk_init_obj.Spi45ClockSelection = spi_clk_src[num-1];
	else               spi_clk_init_obj.Spi6ClockSelection = spi_clk_src[num-1];
	HAL_RCCEx_PeriphCLKConfig(&spi_clk_init_obj);
#endif

    obj->attr = *attr;
	switch (num) {
	case 1:	__HAL_RCC_SPI1_CLK_ENABLE();	break;
	case 2:	__HAL_RCC_SPI2_CLK_ENABLE();	break;
#if (BSP_CHIP_RESOURCE_LEVEL > 1)
	case 3:	__HAL_RCC_SPI3_CLK_ENABLE();	break;
	case 4:	__HAL_RCC_SPI4_CLK_ENABLE();	break;
	case 5:	__HAL_RCC_SPI5_CLK_ENABLE();	break;
	case 6:	__HAL_RCC_SPI6_CLK_ENABLE();	break;
#endif
	}

	drv_spi_pin_source_init(num, nss, sck, miso, mosi);	

	SPI_TypeDef *spi_instance[6] = {
		SPI1, SPI2, 
#if (BSP_CHIP_RESOURCE_LEVEL > 1)
		SPI3, 
		SPI4, SPI5, SPI6
#endif
	};

//   hspi2.Instance = SPI2;
//   hspi2.Init.Mode = SPI_MODE_MASTER;
//   hspi2.Init.Direction = SPI_DIRECTION_2LINES;
//   hspi2.Init.DataSize = SPI_DATASIZE_16BIT;
//   hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
//   hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
//   hspi2.Init.NSS = SPI_NSS_SOFT;
//   hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
//   hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
//   hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
//   hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
//   hspi2.Init.CRCPolynomial = 10;
//   if (HAL_SPI_Init(&hspi2) != HAL_OK)
//   {
//     //Error_Handler();
//   }

	obj->hspi.Instance               = spi_instance[num-1];
    obj->hspi.Init.BaudRatePrescaler = obj->attr.speed;
	obj->hspi.Init.Mode              = SPI_MODE_MASTER;
	obj->hspi.Init.Direction         = SPI_DIRECTION_2LINES;
	obj->hspi.Init.DataSize          = obj->attr.databits;
	switch (obj->attr.mode) {
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
#if defined (DRV_BSP_H7)
	obj->hspi.Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;
	// SPI_MASTER_KEEP_IO_STATE_DISABLE SPI_MASTER_KEEP_IO_STATE_ENABLE
	obj->hspi.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
#endif
	obj->hspi.Init.FirstBit          = SPI_FIRSTBIT_MSB;
	obj->hspi.Init.TIMode            = SPI_TIMODE_DISABLE;
	obj->hspi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    obj->hspi.Init.CRCPolynomial     = 10;
#if defined (DRV_BSP_H7)
    obj->hspi.Init.FifoThreshold     = SPI_FIFO_THRESHOLD_01DATA;
#endif

    __HAL_SPI_DISABLE(&obj->hspi);	
	HAL_SPI_DeInit(&obj->hspi);
	HAL_SPI_Init(&obj->hspi);
	__HAL_SPI_ENABLE(&obj->hspi);

	drv_spi_list[num-1] = obj;
}

int drv_spi_write(struct drv_spi_t *obj, const uint8_t *p, uint16_t len, enum __drv_rwway way)
{
	return HAL_SPI_Transmit(&obj->hspi, p, len, 5000);
}

int drv_spi_read(struct drv_spi_t *obj, uint8_t *p, uint16_t len, enum __drv_rwway way)
{
	return HAL_SPI_Receive(&obj->hspi, p, len, 5000);
}

int drv_spi_readwrite(struct drv_spi_t *obj, uint8_t *ptx, uint8_t *prx, uint16_t size, enum __drv_rwway way)
{
    int8_t res = HAL_SPI_TransmitReceive(&obj->hspi, ptx, prx, size, 5000);
	return res;  
}
