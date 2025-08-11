#include "drv_spi.h"

struct spi_dev_s *g_spi_list[DRV_SPI_PERIPHAL_NUM];

static bool low_pinconfig(struct spi_dev_s *dev);
static void low_dmatx_setup(struct spi_dev_s *dev);
static void low_dmarx_setup(struct spi_dev_s *dev);
static void low_setup(struct spi_dev_s *dev);
static void low_irq(struct spi_dev_s *dev);
static void low_irq_dmatx(struct spi_dev_s *dev);
static void low_irq_dmarx(struct spi_dev_s *dev);
static void low_irq_dmartx(struct spi_dev_s *dev);

static int up_setup(struct spi_dev_s *dev);
static uint32_t up_setfrequency(struct spi_dev_s *dev, uint32_t frequency);
static void up_setmode(struct spi_dev_s *dev, enum spi_mode_e mode);
static void up_setbits(struct spi_dev_s *dev, int nbits);
static int up_lock(struct spi_dev_s *dev, bool lock);
static int up_select(struct spi_dev_s *dev, uint32_t devid, bool selected);
static int up_exchange(struct spi_dev_s *dev,
	const void *txbuffer, void *rxbuffer, size_t nwords);
static int up_exchangeblock(struct spi_dev_s *dev,
	const void *txbuffer, void *rxbuffer, size_t nwords);
static int up_sndblock(struct spi_dev_s *dev, const void *buffer, size_t nwords);
static int up_recvblock(struct spi_dev_s *dev, void *buffer, size_t nwords);
const struct spi_ops_s g_spi_ops = 
{
    .setup = up_setup,
    .setfrequency = up_setfrequency,
	.setmode = up_setmode,
	.setbits = up_setbits,
	.lock = up_lock,
	.select = up_select,
	.exchange = up_exchange,
	.sndblock = up_sndblock,
	.recvblock = up_recvblock,
};

/****************************************************************************
 * Private Function
 ****************************************************************************/
bool low_pinconfig(struct spi_dev_s *dev)
{
    struct up_spi_dev_s *priv = dev->priv;
    uint8_t num = priv->id;
    uint8_t pin_ncs = priv->pin_ncs;
    uint8_t pin_sck = priv->pin_sck;
    uint8_t pin_miso = priv->pin_miso;
    uint8_t pin_mosi = priv->pin_mosi;

#if defined (DRV_BSP_H7)
	const struct pin_node *nss_node;
	const struct pin_node *sck_node;
	const struct pin_node *miso_node;
	const struct pin_node *mosi_node;
	uint32_t illegal;

	switch (num) {
	case 1:
		if (PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(1, SPI_PIN_NSS, pin_ncs)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(1, SPI_PIN_SCK, pin_sck)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(1, SPI_PIN_MISO, pin_miso)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(1, SPI_PIN_MOSI, pin_mosi)) != NULL) {
			nss_node = SPI_PINCTRL_SOURCE(1, SPI_PIN_NSS, pin_ncs);
			sck_node = SPI_PINCTRL_SOURCE(1, SPI_PIN_SCK, pin_sck);
			miso_node = SPI_PINCTRL_SOURCE(1, SPI_PIN_MISO, pin_miso);
			mosi_node = SPI_PINCTRL_SOURCE(1, SPI_PIN_MOSI, pin_mosi);
			illegal = nss_node->port && nss_node->port
						&& nss_node->port && nss_node->port;
		}else {
			return false;
		}
		break;
	case 2:
		if (PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(2, SPI_PIN_NSS, pin_ncs)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(2, SPI_PIN_SCK, pin_sck)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(2, SPI_PIN_MISO, pin_miso)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(2, SPI_PIN_MOSI, pin_mosi)) != NULL) {
			nss_node = SPI_PINCTRL_SOURCE(2, SPI_PIN_NSS, pin_ncs);
			sck_node = SPI_PINCTRL_SOURCE(2, SPI_PIN_SCK, pin_sck);
			miso_node = SPI_PINCTRL_SOURCE(2, SPI_PIN_MISO, pin_miso);
			mosi_node = SPI_PINCTRL_SOURCE(2, SPI_PIN_MOSI, pin_mosi);
			illegal = nss_node->port && nss_node->port
						&& nss_node->port && nss_node->port;
		}else {
			return false;
		}
		break;
	case 3:
		if (PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(3, SPI_PIN_NSS, pin_ncs)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(3, SPI_PIN_SCK, pin_sck)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(3, SPI_PIN_MISO, pin_miso)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(3, SPI_PIN_MOSI, pin_mosi)) != NULL) {
			nss_node = SPI_PINCTRL_SOURCE(3, SPI_PIN_NSS, pin_ncs);
			sck_node = SPI_PINCTRL_SOURCE(3, SPI_PIN_SCK, pin_sck);
			miso_node = SPI_PINCTRL_SOURCE(3, SPI_PIN_MISO, pin_miso);
			mosi_node = SPI_PINCTRL_SOURCE(3, SPI_PIN_MOSI, pin_mosi);
			illegal = nss_node->port && nss_node->port
						&& nss_node->port && nss_node->port;
		}else {
			return false;
		}
		break;
	case 4:
		if (PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(4, SPI_PIN_NSS, pin_ncs)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(4, SPI_PIN_SCK, pin_sck)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(4, SPI_PIN_MISO, pin_miso)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(4, SPI_PIN_MOSI, pin_mosi)) != NULL) {
			nss_node = SPI_PINCTRL_SOURCE(4, SPI_PIN_NSS, pin_ncs);
			sck_node = SPI_PINCTRL_SOURCE(4, SPI_PIN_SCK, pin_sck);
			miso_node = SPI_PINCTRL_SOURCE(4, SPI_PIN_MISO, pin_miso);
			mosi_node = SPI_PINCTRL_SOURCE(4, SPI_PIN_MOSI, pin_mosi);
			illegal = nss_node->port && nss_node->port
						&& nss_node->port && nss_node->port;
		}else {
			return false;
		}
		break;
	case 5:
		if (PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(5, SPI_PIN_NSS, pin_ncs)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(5, SPI_PIN_SCK, pin_sck)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(5, SPI_PIN_MISO, pin_miso)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(5, SPI_PIN_MOSI, pin_mosi)) != NULL) {
			nss_node = SPI_PINCTRL_SOURCE(5, SPI_PIN_NSS, pin_ncs);
			sck_node = SPI_PINCTRL_SOURCE(5, SPI_PIN_SCK, pin_sck);
			miso_node = SPI_PINCTRL_SOURCE(5, SPI_PIN_MISO, pin_miso);
			mosi_node = SPI_PINCTRL_SOURCE(5, SPI_PIN_MOSI, pin_mosi);
			illegal = nss_node->port && nss_node->port
						&& nss_node->port && nss_node->port;
		}else {
			return false;
		}
		break;
	case 6:
		if (PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(6, SPI_PIN_NSS, pin_ncs)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(6, SPI_PIN_SCK, pin_sck)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(6, SPI_PIN_MISO, pin_miso)) != NULL &&
			PINNODE(uint32_t)(SPI_PINCTRL_SOURCE(6, SPI_PIN_MOSI, pin_mosi)) != NULL) {
			nss_node = SPI_PINCTRL_SOURCE(6, SPI_PIN_NSS, pin_ncs);
			sck_node = SPI_PINCTRL_SOURCE(6, SPI_PIN_SCK, pin_sck);
			miso_node = SPI_PINCTRL_SOURCE(6, SPI_PIN_MISO, pin_miso);
			mosi_node = SPI_PINCTRL_SOURCE(6, SPI_PIN_MOSI, pin_mosi);
			illegal = nss_node->port && nss_node->port
						&& nss_node->port && nss_node->port;
		}else {
			return false;
		}
		break;
	default: return false;
	}
	
	if (illegal != 0) {
        // LOW_PERIPH_INITPIN(nss_node->port, nss_node->pin_num, IOMODE_AFPP, IO_PULLUP, IO_SPEEDMAX, nss_node->alternate);
		LOW_PERIPH_INITPIN(sck_node->port, sck_node->pin, IOMODE_AFPP, IO_PULLUP, IO_SPEEDMAX, sck_node->alternate);
		LOW_PERIPH_INITPIN(miso_node->port, miso_node->pin, IOMODE_AFPP, IO_PULLUP, IO_SPEEDMAX, miso_node->alternate);
		LOW_PERIPH_INITPIN(mosi_node->port, mosi_node->pin, IOMODE_AFPP, IO_PULLUP, IO_SPEEDMAX, mosi_node->alternate);
	}else {
		return false;
	}
	return true;
#endif

#if defined (DRV_BSP_F1)
    if (pin_ncs == 0) {
        GPIO_TypeDef *spi_port[3] =  { GPIOA,		GPIOB,      GPIOB };
        uint16_t       sck_pin[3] =  {   5,         13,          3    };
        uint16_t       miso_pin[3] = {   6,         14,          4    };
		uint16_t       mosi_pin[3] = {   7,         15,          5    };
		LOW_PERIPH_INITPIN(spi_port[num-1], sck_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH);
		LOW_PERIPH_INITPIN(spi_port[num-1], mosi_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH);
		LOW_PERIPH_INITPIN(spi_port[num-1], miso_pin[num-1], IOMODE_INPUT, IO_NOPULL, IO_SPEEDHIGH);
    } else if (pin_ncs == 1 && num == 1) {
		LOW_PERIPH_INITPIN(GPIOB, 3, IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH);  //sck
		LOW_PERIPH_INITPIN(GPIOB, 4, IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH);  //miso
		LOW_PERIPH_INITPIN(GPIOB, 5, IOMODE_INPUT, IO_NOPULL, IO_SPEEDHIGH);  //mosi  
    }
	return true;
#endif

#if defined (DRV_BSP_F4)
    if (pin_ncs == 0) {
        GPIO_TypeDef *spi_port[3] =  { GPIOA,		GPIOB,      GPIOB };
        uint16_t       sck_pin[3] =  {   5,         13,          3    };
        uint16_t       miso_pin[3] = {   6,         14,          4    };
		uint16_t       mosi_pin[3] = {   7,         15,          5    };
        uint32_t       alternate[3] = {GPIO_AF5_SPI1, GPIO_AF5_SPI2, GPIO_AF6_SPI3};

		LOW_PERIPH_INITPIN(spi_port[num-1], sck_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, alternate[num-1]);
		LOW_PERIPH_INITPIN(spi_port[num-1], mosi_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, alternate[num-1]);
		LOW_PERIPH_INITPIN(spi_port[num-1], miso_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, alternate[num-1]);
    } else if (pin_ncs == 1) {
        GPIO_TypeDef *spi_port[3] =  { GPIOB,		GPIOB,      GPIOC };
        uint16_t       sck_pin[3] =  {   3,         10,          10   };
        uint16_t       miso_pin[3] = {   4,         2,          11    };
		uint16_t       mosi_pin[3] = {   5,         3,          12    };
        uint32_t       alternate[3] = {GPIO_AF5_SPI1, GPIO_AF5_SPI2, GPIO_AF6_SPI3};

		LOW_PERIPH_INITPIN(spi_port[num-1], sck_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, alternate[num-1]);
		if (num == 2) {
			spi_port[num-1] = GPIOC;
		}
		LOW_PERIPH_INITPIN(spi_port[num-1], mosi_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, alternate[num-1]);
		LOW_PERIPH_INITPIN(spi_port[num-1], miso_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, alternate[num-1]);
    }
	return true;
#endif
}

void low_dmatx_setup(struct spi_dev_s *dev)
{
    struct up_spi_dev_s *priv = dev->priv;
    uint8_t num = priv->id;

#if defined (DRV_BSP_H7)
	DMA_Stream_TypeDef *spi_txdma_stream[DRV_SPI_PERIPHAL_NUM] = {
        DMA1_Stream2, DMA1_Stream3, DMA1_Stream4,
        DMA1_Stream5, DMA1_Stream6, BDMA_Channel0
    };
	uint8_t spi_txdma_request[DRV_SPI_PERIPHAL_NUM] = {
        DMA_REQUEST_SPI1_TX, DMA_REQUEST_SPI2_TX, 
		DMA_REQUEST_SPI3_TX, DMA_REQUEST_SPI4_TX, 
        DMA_REQUEST_SPI5_TX, BDMA_REQUEST_SPI6_TX
    };
    IRQn_Type spi_txdma_irq[DRV_SPI_PERIPHAL_NUM] = {
        DMA1_Stream2_IRQn, DMA1_Stream3_IRQn, DMA1_Stream4_IRQn,
        DMA1_Stream5_IRQn, DMA1_Stream6_IRQn, BDMA_Channel0_IRQn
    };
#elif defined(DRV_BSP_F4)
	DMA_Stream_TypeDef *spi_txdma_stream[DRV_SPI_PERIPHAL_NUM] = {
        DMA2_Stream3, DMA1_Stream4, DMA1_Stream5,
        DMA2_Stream1, DMA2_Stream4, DMA2_Stream5
    };
    uint32_t spi_txdma_channel[DRV_SPI_PERIPHAL_NUM] = {
        DMA_CHANNEL_3, DMA_CHANNEL_0, DMA_CHANNEL_0,
        DMA_CHANNEL_4, DMA_CHANNEL_2, DMA_CHANNEL_1
    };
    IRQn_Type spi_txdma_irq[DRV_SPI_PERIPHAL_NUM] = {
        DMA2_Stream3_IRQn, DMA1_Stream4_IRQn, DMA1_Stream5_IRQn,
        DMA2_Stream1_IRQn, DMA2_Stream4_IRQn, DMA2_Stream5_IRQn
    };
#endif  // End With Define DRV_BSP_F4, DRV_BSP_H7

    __HAL_LINKDMA(&priv->hspi, hdmatx, priv->txdma);
#if defined (DRV_BSP_H7)
	if (num != 6) {
		__HAL_RCC_DMA1_CLK_ENABLE();
	} else {
		__HAL_RCC_BDMA_CLK_ENABLE();
	}
	//HAL_DMA_DeInit(&obj->txdma);
	priv->txdma.Instance = spi_txdma_stream[num-1];
	priv->txdma.Init.Request = spi_txdma_request[num-1];
	priv->txdma.Init.Direction = DMA_MEMORY_TO_PERIPH;
	priv->txdma.Init.PeriphInc = DMA_PINC_DISABLE;
	priv->txdma.Init.MemInc = DMA_MINC_ENABLE;
	priv->txdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	priv->txdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	priv->txdma.Init.Mode = DMA_NORMAL;
	priv->txdma.Init.Priority = DMA_PRIORITY_HIGH;
	priv->txdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	priv->txdma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	priv->txdma.Init.MemBurst = DMA_MBURST_SINGLE;
	priv->txdma.Init.PeriphBurst = DMA_PBURST_SINGLE;
#elif defined(DRV_BSP_F4)
	if (num == 2 || num == 3) {
		__HAL_RCC_DMA1_CLK_ENABLE();
	} else if (num == 1 || num == 4 || num == 5 || num ==6) {
		__HAL_RCC_DMA2_CLK_ENABLE();
	}
	priv->txdma.Instance = spi_txdma_stream[num-1];
	priv->txdma.Init.Channel = spi_txdma_channel[num-1];
	priv->txdma.Init.Direction = DMA_MEMORY_TO_PERIPH;
	priv->txdma.Init.PeriphInc = DMA_PINC_DISABLE;
	priv->txdma.Init.MemInc = DMA_MINC_ENABLE;
	priv->txdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	priv->txdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	priv->txdma.Init.Mode = DMA_NORMAL;
	priv->txdma.Init.Priority = DMA_PRIORITY_HIGH;
	priv->txdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
#endif
	HAL_DMA_Init(&priv->txdma);
	HAL_NVIC_SetPriority(spi_txdma_irq[num-1], priv->priority_dmatx, 0);
	HAL_NVIC_EnableIRQ(spi_txdma_irq[num-1]);
}

void low_dmarx_setup(struct spi_dev_s *dev)
{
    struct up_spi_dev_s *priv = dev->priv;
    uint8_t num = priv->id;

#if defined (DRV_BSP_H7)
	DMA_Stream_TypeDef *spi_rxdma_stream[DRV_SPI_PERIPHAL_NUM] = {
        DMA2_Stream2, DMA2_Stream3, DMA2_Stream4,
        DMA2_Stream5, DMA2_Stream6, BDMA_Channel1
    };
	uint8_t spi_rxdma_request[DRV_SPI_PERIPHAL_NUM] = {
        DMA_REQUEST_SPI1_RX, DMA_REQUEST_SPI2_RX, 
		DMA_REQUEST_SPI3_RX, DMA_REQUEST_SPI4_RX, 
        DMA_REQUEST_SPI5_RX, BDMA_REQUEST_SPI6_RX
    };
    IRQn_Type spi_rxdma_irq[DRV_SPI_PERIPHAL_NUM] = {
        DMA2_Stream2_IRQn, DMA2_Stream3_IRQn, DMA2_Stream4_IRQn,
        DMA2_Stream5_IRQn, DMA2_Stream6_IRQn, BDMA_Channel1_IRQn
    };
#elif defined(DRV_BSP_F4)
	DMA_Stream_TypeDef *spi_rxdma_stream[DRV_SPI_PERIPHAL_NUM] = {
        DMA2_Stream2, DMA1_Stream3, DMA1_Stream0,
        DMA2_Stream0, DMA2_Stream5, DMA2_Stream6
    };
    uint32_t spi_rxdma_channel[DRV_SPI_PERIPHAL_NUM] = {
        DMA_CHANNEL_3, DMA_CHANNEL_0, DMA_CHANNEL_0,
        DMA_CHANNEL_4, DMA_CHANNEL_7, DMA_CHANNEL_1
    };
    IRQn_Type spi_rxdma_irq[DRV_SPI_PERIPHAL_NUM] = {
        DMA2_Stream2_IRQn, DMA1_Stream3_IRQn, DMA1_Stream0_IRQn,
        DMA2_Stream0_IRQn, DMA2_Stream5_IRQn, DMA2_Stream6_IRQn
    };
#endif  // End With Define DRV_BSP_F4, DRV_BSP_H7

    __HAL_LINKDMA(&priv->hspi, hdmarx, priv->rxdma);
#if defined (DRV_BSP_H7)
    if (num != 6) {
		__HAL_RCC_DMA2_CLK_ENABLE();
	} else {
		__HAL_RCC_BDMA_CLK_ENABLE();
	}
    //HAL_DMA_DeInit(&obj->rxdma);
    priv->rxdma.Instance = spi_rxdma_stream[num-1];
    priv->rxdma.Init.Request = spi_rxdma_request[num-1];
    priv->rxdma.Init.Direction = DMA_PERIPH_TO_MEMORY;
    priv->rxdma.Init.PeriphInc = DMA_PINC_DISABLE;
    priv->rxdma.Init.MemInc = DMA_MINC_ENABLE;
    priv->rxdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    priv->rxdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    priv->rxdma.Init.Mode = DMA_NORMAL;
    priv->rxdma.Init.Priority = DMA_PRIORITY_HIGH;
    priv->rxdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    priv->rxdma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    priv->rxdma.Init.MemBurst = DMA_MBURST_SINGLE;
    priv->rxdma.Init.PeriphBurst = DMA_PBURST_SINGLE;
#elif defined(DRV_BSP_F4)
	if (num == 2 || num == 3) {
		__HAL_RCC_DMA1_CLK_ENABLE();
	} else if (num == 1 || num == 4 || num == 5 || num ==6) {
		__HAL_RCC_DMA2_CLK_ENABLE();
	}
	priv->rxdma.Instance = spi_rxdma_stream[num-1];
	priv->rxdma.Init.Channel = spi_rxdma_channel[num-1];
	priv->rxdma.Init.Direction = DMA_PERIPH_TO_MEMORY;
	priv->rxdma.Init.PeriphInc = DMA_PINC_DISABLE;
	priv->rxdma.Init.MemInc = DMA_MINC_ENABLE;
	priv->rxdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	priv->rxdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	priv->rxdma.Init.Mode = DMA_NORMAL;
	priv->rxdma.Init.Priority = DMA_PRIORITY_HIGH;
	priv->rxdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
#endif
	HAL_DMA_Init(&priv->rxdma);
	HAL_NVIC_SetPriority(spi_rxdma_irq[num-1], priv->priority_dmarx, 0);
	HAL_NVIC_EnableIRQ(spi_rxdma_irq[num-1]);
}

void low_setup(struct spi_dev_s *dev)
{
    struct up_spi_dev_s *priv = dev->priv;
    uint8_t num = priv->id;
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

	switch (num) {
	case 1:	__HAL_RCC_SPI1_CLK_ENABLE();	break;
	case 2:	__HAL_RCC_SPI2_CLK_ENABLE();	break;
#if (BSP_CHIP_RESOURCE_LEVEL > 1)
	case 3:	__HAL_RCC_SPI3_CLK_ENABLE();	break;
#elif (BSP_CHIP_RESOURCE_LEVEL > 2)
	case 4:	__HAL_RCC_SPI4_CLK_ENABLE();	break;
	case 5:	__HAL_RCC_SPI5_CLK_ENABLE();	break;
	case 6:	__HAL_RCC_SPI6_CLK_ENABLE();	break;
#endif
	}

	low_pinconfig(dev);	

	SPI_TypeDef *spi_instance[6] = {
		SPI1, SPI2, 
#if (BSP_CHIP_RESOURCE_LEVEL > 1)
		SPI3,
#elif (BSP_CHIP_RESOURCE_LEVEL > 2)
		SPI4, SPI5, SPI6
#endif
	};

	IRQn_Type spi_irq_array[6] = {
		SPI1_IRQn, SPI2_IRQn, 
#if (BSP_CHIP_RESOURCE_LEVEL > 1)
		SPI3_IRQn, 
#elif (BSP_CHIP_RESOURCE_LEVEL > 2)
		SPI4_IRQn, SPI5_IRQn, SPI6_IRQn
#endif
    };

	priv->hspi.Instance               = spi_instance[num-1];
    priv->hspi.Init.BaudRatePrescaler = dev->frequency;
	priv->hspi.Init.Mode              = SPI_MODE_MASTER;
	priv->hspi.Init.Direction         = SPI_DIRECTION_2LINES;
	switch (dev->nbits) {
	case 8:
        priv->hspi.Init.DataSize = SPI_DATASIZE_8BIT;
        break;
	case 16:
        priv->hspi.Init.DataSize = SPI_DATASIZE_16BIT;
        break;
	}
	switch (dev->mode) {
	case SPIDEV_MODE0:
        priv->hspi.Init.CLKPolarity       = SPI_POLARITY_LOW;
		priv->hspi.Init.CLKPhase          = SPI_PHASE_1EDGE;
		break;
	case SPIDEV_MODE1:
        priv->hspi.Init.CLKPolarity       = SPI_POLARITY_LOW;
        priv->hspi.Init.CLKPhase          = SPI_PHASE_2EDGE;
		break;
	case SPIDEV_MODE2:
        priv->hspi.Init.CLKPolarity       = SPI_POLARITY_HIGH;
        priv->hspi.Init.CLKPhase          = SPI_PHASE_1EDGE;
		break;
	case SPIDEV_MODE3:
        priv->hspi.Init.CLKPolarity       = SPI_POLARITY_HIGH;
        priv->hspi.Init.CLKPhase          = SPI_PHASE_2EDGE;
		break;	
	default :break;
	}
	priv->hspi.Init.NSS               = SPI_NSS_SOFT;
#if defined (DRV_BSP_H7)
    priv->hspi.Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;
    priv->hspi.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
	priv->hspi.Init.MasterSSIdleness  = SPI_MASTER_SS_IDLENESS_00CYCLE;
	priv->hspi.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_10CYCLE;
#endif
    priv->hspi.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    priv->hspi.Init.TIMode            = SPI_TIMODE_DISABLE;
    priv->hspi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    priv->hspi.Init.CRCPolynomial     = 10;
#if defined (DRV_BSP_H7)
    priv->hspi.Init.FifoThreshold     = SPI_FIFO_THRESHOLD_01DATA;
#endif
    __HAL_SPI_DISABLE(&priv->hspi);	
	HAL_SPI_DeInit(&priv->hspi);
	HAL_SPI_Init(&priv->hspi);
	__HAL_SPI_ENABLE(&priv->hspi);

    if (priv->enable_dmarx) {
        low_dmarx_setup(dev);
    }
    if (priv->enable_dmatx) {
        low_dmatx_setup(dev);
    }
	HAL_NVIC_SetPriority(spi_irq_array[num-1], priv->priority, 0);
	HAL_NVIC_EnableIRQ(spi_irq_array[num-1]);

	g_spi_list[num-1] = dev;
}

void low_irq(struct spi_dev_s *dev)
{
	struct up_spi_dev_s *priv = dev->priv;
	HAL_SPI_IRQHandler(&priv->hspi);
}

void low_irq_dmatx(struct spi_dev_s *dev)
{
	struct up_spi_dev_s *priv = dev->priv;
	dev->txresult = 0x01;
    spi_dmatxwakeup(dev);
}

void low_irq_dmarx(struct spi_dev_s *dev)
{
	struct up_spi_dev_s *priv = dev->priv;
    dev->rxresult = 0x01;
    spi_dmarxwakeup(dev);
}

void low_irq_dmartx(struct spi_dev_s *dev)
{
	struct up_spi_dev_s *priv = dev->priv;
    dev->txresult = 0x01;
	spi_dmatxwakeup(dev);
}

/****************************************************************************
 * Public Function Interface 
 ****************************************************************************/
int up_setup(struct spi_dev_s *dev)
{
    low_setup(dev);
    return 0;
}

uint32_t up_setfrequency(struct spi_dev_s *dev, uint32_t frequency)
{
    struct up_spi_dev_s *priv = dev->priv;
#if defined (DRV_BSP_H7)
	priv->hspi.Instance->CFG1 |= (frequency << 28);
#endif

#if defined (DRV_BSP_F4) || defined (DRV_BSP_F1)
    priv->hspi.Instance->CR1 |= (frequency << 3);
#endif

	return 0;
}

void up_setmode(struct spi_dev_s *dev, enum spi_mode_e mode)
{
    struct up_spi_dev_s *priv = dev->priv;
#if defined (DRV_BSP_H7)
    switch (dev->mode) {
	case SPIDEV_MODE0:
        priv->hspi.Instance->CFG2 |= SPI_POLARITY_LOW | SPI_PHASE_1EDGE;
		break;
	case SPIDEV_MODE1:
        priv->hspi.Instance->CFG2 |= SPI_POLARITY_LOW | SPI_PHASE_2EDGE;
		break;
	case SPIDEV_MODE2:
        priv->hspi.Instance->CFG2 |= SPI_POLARITY_HIGH | SPI_PHASE_1EDGE;
		break;
	case SPIDEV_MODE3:
        priv->hspi.Instance->CFG2 |= SPI_POLARITY_HIGH | SPI_PHASE_2EDGE;
		break;	
	default :break;
	}
#endif

#if defined (DRV_BSP_F4) || defined (DRV_BSP_F1)
    switch (dev->mode) {
	case SPIDEV_MODE0:
        priv->hspi.Instance->CR1 |= SPI_POLARITY_LOW | SPI_PHASE_1EDGE;
		break;
	case SPIDEV_MODE1:
        priv->hspi.Instance->CR1 |= SPI_POLARITY_LOW | SPI_PHASE_2EDGE;
		break;
	case SPIDEV_MODE2:
        priv->hspi.Instance->CR1 |= SPI_POLARITY_HIGH | SPI_PHASE_1EDGE;
		break;
	case SPIDEV_MODE3:
        priv->hspi.Instance->CR1 |= SPI_POLARITY_HIGH | SPI_PHASE_2EDGE;
		break;	
	default :break;
	}
#endif

}

void up_setbits(struct spi_dev_s *dev, int nbits)
{
    struct up_spi_dev_s *priv = dev->priv;
#if defined (DRV_BSP_H7)
    switch (nbits) {
	case 8:
        priv->hspi.Instance->CFG1 |= SPI_DATASIZE_8BIT;
		break;
	case 16:
        priv->hspi.Instance->CFG1 |= SPI_DATASIZE_16BIT;
		break;
	default :break;
	}
#endif

#if defined (DRV_BSP_F4) || defined (DRV_BSP_F1)
	priv->hspi.Instance->CR1 &= ~SPI_CR1_SPE;  // disable spi
    switch (nbits) {
	case 8:
        priv->hspi.Instance->CR1 |= (0x0UL) << 11;
		break;
	case 16:
        priv->hspi.Instance->CR1 |= (0x1UL) << 11;
		break;
	default :break;
	}
	priv->hspi.Instance->CR1 |= SPI_CR1_SPE;  // enable spi
#endif
}

int up_lock(struct spi_dev_s *dev, bool lock)
{
	return spi_devlock(dev, lock);
}

int up_select(struct spi_dev_s *dev, uint32_t devid, bool selected)
{
    struct up_spi_dev_s *priv = dev->priv;
	int i = 0;
	for (; i < CONFIG_SPI_ATTACH_CS_NUM; i++) {
        if (devid == priv->devid[i] && priv->devcs[i].port != NULL) {
			HAL_GPIO_WritePin(priv->devcs[i].port, (0x01 << priv->devcs[i].pin), !selected);
			return 0;
		}
	}
	return 1;
}

int up_exchange(struct spi_dev_s *dev, const void *txbuffer, void *rxbuffer, size_t nwords)
{
    int ret = 0;
    struct up_spi_dev_s *priv = dev->priv;

	// bug: when write sensor register data:
	// call HAL_SPI_TransmitReceive_DMA(dev, {addr, data}, {0x00, 0x00}, 2), it can not work
	// call HAL_SPI_Transmit_DMA(dev, {addr, data}, 2), it works
	// ???

	if (!txbuffer && rxbuffer) {
		dev->rxresult = 0x00;
		ret = HAL_SPI_Receive_DMA(&priv->hspi, rxbuffer, nwords);
        ret = spi_dmarxwait(dev);
	} else if (txbuffer && !rxbuffer) {
		dev->txresult = 0x00;
		ret = HAL_SPI_Transmit_DMA(&priv->hspi, txbuffer, nwords);
        ret = spi_dmatxwait(dev);
	} else if (txbuffer && rxbuffer) {
		dev->txresult = 0x00;
		ret = HAL_SPI_TransmitReceive_DMA(&priv->hspi, txbuffer, rxbuffer, nwords);
        ret = spi_dmatxwait(dev);
	}

	return ret;
}

int up_exchangeblock(struct spi_dev_s *dev,
	const void *txbuffer, void *rxbuffer, size_t nwords)
{
	int ret = 0;
	struct up_spi_dev_s *priv = dev->priv;

	ret = HAL_SPI_TransmitReceive(&priv->hspi, txbuffer, rxbuffer, nwords, 1000);
	return ret;
}

int up_sndblock(struct spi_dev_s *dev, const void *buffer, size_t nwords)
{
	int ret = 0;
	struct up_spi_dev_s *priv = dev->priv;

	ret = HAL_SPI_Transmit(&priv->hspi, buffer, nwords, 1000);
	return ret;
}

int up_recvblock(struct spi_dev_s *dev, void *buffer, size_t nwords)
{
	int ret = 0;
	struct up_spi_dev_s *priv = dev->priv;

	ret = HAL_SPI_Receive(&priv->hspi, buffer, nwords, 1000);
	return ret;
}

/****************************************************************************
 * STM32 HAL Library Callback 
 ****************************************************************************/

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    uint8_t idx = 0;
    if (hspi->Instance == SPI1)		    idx = 0;
    else if (hspi->Instance == SPI2)	idx = 1;
#if (BSP_CHIP_RESOURCE_LEVEL > 1)
    else if (hspi->Instance == SPI3)	idx = 2;
#elif (BSP_CHIP_RESOURCE_LEVEL > 2)
    else if (hspi->Instance == SPI4)	idx = 3;
    else if (hspi->Instance == SPI5)	idx = 4;
    else if (hspi->Instance == SPI6)	idx = 5;
#endif

    low_irq_dmartx(g_spi_list[idx]);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    uint8_t idx = 0;
    if (hspi->Instance == SPI1)		    idx = 0;
    else if (hspi->Instance == SPI2)	idx = 1;
#if (BSP_CHIP_RESOURCE_LEVEL > 1)
    else if (hspi->Instance == SPI3)	idx = 2;
#elif (BSP_CHIP_RESOURCE_LEVEL > 2)
    else if (hspi->Instance == SPI4)	idx = 3;
    else if (hspi->Instance == SPI5)	idx = 4;
    else if (hspi->Instance == SPI6)	idx = 5;
#endif

    low_irq_dmatx(g_spi_list[idx]);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    uint8_t idx = 0;
    if (hspi->Instance == SPI1)		    idx = 0;
    else if (hspi->Instance == SPI2)	idx = 1;
#if (BSP_CHIP_RESOURCE_LEVEL > 1)
    else if (hspi->Instance == SPI3)	idx = 2;
#elif (BSP_CHIP_RESOURCE_LEVEL > 2)
    else if (hspi->Instance == SPI4)	idx = 3;
    else if (hspi->Instance == SPI5)	idx = 4;
    else if (hspi->Instance == SPI6)	idx = 5;
#endif

    low_irq_dmarx(g_spi_list[idx]);
}

void SPI1_IRQHandler(void)
{
	low_irq(g_spi_list[0]);
}

void SPI2_IRQHandler(void)
{
	low_irq(g_spi_list[1]);
}

#if (BSP_CHIP_RESOURCE_LEVEL > 1)
void SPI3_IRQHandler(void)
{
	low_irq(g_spi_list[2]);
}

void SPI4_IRQHandler(void)
{
	low_irq(g_spi_list[3]);
}

void SPI5_IRQHandler(void)
{
	low_irq(g_spi_list[4]);
}

void SPI6_IRQHandler(void)
{
	low_irq(g_spi_list[5]);
}
#endif
