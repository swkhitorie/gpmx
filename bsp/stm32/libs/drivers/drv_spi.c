#include "drv_spi.h"

// #define DRV_SPI_PERIPHAL_NUM   CONFIG_SPI_PERIPHAL_NUM
// static struct spi_dev_s *spi_list[DRV_SPI_PERIPHAL_NUM];

static bool low_pinconfig(struct spi_dev_s *dev);
static void low_setup(struct spi_dev_s *dev);

static int up_setup(struct spi_dev_s *dev);
static uint32_t up_setfrequency(struct spi_dev_s *dev, uint32_t frequency);
static void up_setmode(struct spi_dev_s *dev, enum spi_mode_e mode);
static void up_setbits(struct spi_dev_s *dev, int nbits);
static int up_lock(struct spi_dev_s *dev, bool lock);
static int up_select(struct spi_dev_s *dev, uint32_t devid, bool selected);
static int up_exchange(struct spi_dev_s *dev,
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
        // low_gpio_setup(nss_node->port, nss_node->pin_num, IOMODE_AFPP, IO_PULLUP, IO_SPEEDMAX, nss_node->alternate, NULL, 0);
		low_gpio_setup(sck_node->port, sck_node->pin, IOMODE_AFPP, IO_PULLUP, IO_SPEEDMAX, sck_node->alternate, NULL, 0);
		low_gpio_setup(miso_node->port, miso_node->pin, IOMODE_AFPP, IO_PULLUP, IO_SPEEDMAX, miso_node->alternate, NULL, 0);
		low_gpio_setup(mosi_node->port, mosi_node->pin, IOMODE_AFPP, IO_PULLUP, IO_SPEEDMAX, mosi_node->alternate, NULL, 0);
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
		low_gpio_setup(spi_port[num-1], sck_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, 0, NULL, 0);
		low_gpio_setup(spi_port[num-1], mosi_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, 0, NULL, 0);
		low_gpio_setup(spi_port[num-1], miso_pin[num-1], IOMODE_INPUT, IO_NOPULL, IO_SPEEDHIGH, 0, NULL, 0);
    } else if (pin_ncs == 1 && num == 1) {
		low_gpio_setup(GPIOB, 3, IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, 0, NULL, 0);  //sck
		low_gpio_setup(GPIOB, 4, IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, 0, NULL, 0);  //miso
		low_gpio_setup(GPIOB, 5, IOMODE_INPUT, IO_NOPULL, IO_SPEEDHIGH, 0, NULL, 0);  //mosi  
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

		low_gpio_setup(spi_port[num-1], sck_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, alternate[num-1], NULL, 0);
		low_gpio_setup(spi_port[num-1], mosi_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, alternate[num-1], NULL, 0);
		low_gpio_setup(spi_port[num-1], miso_pin[num-1], IOMODE_INPUT, IO_NOPULL, IO_SPEEDHIGH, alternate[num-1], NULL, 0);
    } else if (pin_ncs == 1) {
        GPIO_TypeDef *spi_port[3] =  { GPIOB,		GPIOB,      GPIOC };
        uint16_t       sck_pin[3] =  {   3,         10,          10   };
        uint16_t       miso_pin[3] = {   4,         2,          11    };
		uint16_t       mosi_pin[3] = {   5,         3,          12    };
        uint32_t       alternate[3] = {GPIO_AF5_SPI1, GPIO_AF5_SPI2, GPIO_AF6_SPI3};

		low_gpio_setup(spi_port[num-1], sck_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, alternate[num-1], NULL, 0);
		if (num == 2) {
			spi_port[num-1] = GPIOC;
		}
		low_gpio_setup(spi_port[num-1], mosi_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, alternate[num-1], NULL, 0);
		low_gpio_setup(spi_port[num-1], miso_pin[num-1], IOMODE_INPUT, IO_NOPULL, IO_SPEEDHIGH, alternate[num-1], NULL, 0);
    }
	return true;
#endif
}

/****************************************************************************
 * Public Function Interface 
 ****************************************************************************/
int up_setup(struct spi_dev_s *dev)
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
		SPI4, SPI5, SPI6
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
}

uint32_t up_setfrequency(struct spi_dev_s *dev, uint32_t frequency)
{
	return 0;
}

void up_setmode(struct spi_dev_s *dev, enum spi_mode_e mode)
{

}

void up_setbits(struct spi_dev_s *dev, int nbits)
{
    
}

int up_lock(struct spi_dev_s *dev, bool lock)
{
	return 0;
}

int up_select(struct spi_dev_s *dev, uint32_t devid, bool selected)
{
    struct up_spi_dev_s *priv = dev->priv;
	int i = 0;
	for (; i < CONFIG_SPI_ATTACH_CS_NUM; i++) {
        if (devid == priv->devid[i] && priv->devcs[i].port != NULL) {
			HAL_GPIO_WritePin(priv->devcs[i].port, (0x01 << priv->devcs[i].pin), !selected);
		}
	}
}

int up_exchange(struct spi_dev_s *dev, const void *txbuffer, void *rxbuffer, size_t nwords)
{
    struct up_spi_dev_s *priv = dev->priv;
	return HAL_SPI_TransmitReceive(&priv->hspi, txbuffer, rxbuffer, nwords, 5000);
}

int up_sndblock(struct spi_dev_s *dev, const void *buffer, size_t nwords)
{
	struct up_spi_dev_s *priv = dev->priv;
	return HAL_SPI_Transmit(&priv->hspi, buffer, nwords, 5000);
}

int up_recvblock(struct spi_dev_s *dev, void *buffer, size_t nwords)
{
	struct up_spi_dev_s *priv = dev->priv;
	return HAL_SPI_Receive(&priv->hspi, buffer, nwords, 5000);
}
