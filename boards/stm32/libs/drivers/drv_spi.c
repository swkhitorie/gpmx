#include "drv_spi.h"

struct spi_dev_s *g_spi_list[CONFIG_STM32_SPI_NUM];

static uint8_t       _spi_instance_judge(SPI_HandleTypeDef *hspi);
static void          _spi_rcc_init(uint8_t id);
static SPI_TypeDef*  _spi_obj_get(uint8_t id);
static IRQn_Type     _spi_irq_get(uint8_t id);

#if defined(DRV_STM32_H7)
static void     _spi_set_clocksrc(struct spi_dev_s *dev);
static uint32_t _spi_get_clock(struct spi_dev_s *dev);
#endif

static void _spi_pin_config(struct spi_dev_s *dev);
static void _spi_dma_setup(struct spi_dev_s *dev, uint8_t flag);
static void _spi_setup(struct spi_dev_s *dev);
static void _spi_irq(struct spi_dev_s *dev);
static void _spi_irq_dmatx(struct spi_dev_s *dev);
static void _spi_irq_dmarx(struct spi_dev_s *dev);
static void _spi_irq_dmartx(struct spi_dev_s *dev);

static int up_spi_setup(struct spi_dev_s *dev);
static uint32_t up_spi_setfrequency(struct spi_dev_s *dev, uint32_t frequency);
static void up_spi_setmode(struct spi_dev_s *dev, enum spi_mode_e mode);
static void up_spi_setbits(struct spi_dev_s *dev, int nbits);
static int up_spi_lock(struct spi_dev_s *dev, bool lock);
static int up_spi_select(struct spi_dev_s *dev, uint32_t devid, bool selected);
static int up_spi_exchange(struct spi_dev_s *dev,
	const void *txbuffer, void *rxbuffer, size_t nwords);
static int up_spi_exchangeblock(struct spi_dev_s *dev,
	const void *txbuffer, void *rxbuffer, size_t nwords);
static int up_spi_sndblock(struct spi_dev_s *dev, const void *buffer, size_t nwords);
static int up_spi_recvblock(struct spi_dev_s *dev, void *buffer, size_t nwords);
const struct spi_ops_s g_spi_ops = 
{
    .setup = up_spi_setup,
    .setfrequency = up_spi_setfrequency,
	.setmode = up_spi_setmode,
	.setbits = up_spi_setbits,
	.lock = up_spi_lock,
	.select = up_spi_select,
	.exchange = up_spi_exchange,
	.exchangeblock = up_spi_exchangeblock,
	.sndblock = up_spi_sndblock,
	.recvblock = up_spi_recvblock,
};

/****************************************************************************
 * Private Function
 ****************************************************************************/
uint8_t _spi_instance_judge(SPI_HandleTypeDef *hspi)
{
    uint8_t idx=0;
    if      (hspi->Instance == SPI1)    idx = 0;
#if defined(SPI2)
    else if (hspi->Instance == SPI2)    idx = 1;
#endif
#if defined(SPI3)
    else if (hspi->Instance == SPI3)    idx = 2;
#endif
#if defined(SPI4)
    else if (hspi->Instance == SPI4)    idx = 3;
#endif
#if defined(SPI5)
    else if (hspi->Instance == SPI5)    idx = 4;
#endif
#if defined(SPI6)
    else if (hspi->Instance == SPI6)    idx = 5;
#endif
    return idx;
}

void _spi_rcc_init(uint8_t id)
{
    switch (id) {
    case 1:	__HAL_RCC_SPI1_CLK_ENABLE();	break;
#if defined(SPI2)
    case 2:	__HAL_RCC_SPI2_CLK_ENABLE();	break;
#endif
#if defined(SPI3)
    case 3:	__HAL_RCC_SPI3_CLK_ENABLE();	break;
#endif
#if defined(SPI4)
    case 4:	__HAL_RCC_SPI4_CLK_ENABLE();	break;
#endif
#if defined(SPI5)
    case 5:	__HAL_RCC_SPI5_CLK_ENABLE();	break;
#endif
#if defined(SPI6)
    case 6:	__HAL_RCC_SPI6_CLK_ENABLE();	break;
#endif
	}
}

SPI_TypeDef* _spi_obj_get(uint8_t id)
{
    SPI_TypeDef *spi_instance[6] = {
        SPI1, 
#if defined(SPI2)
        SPI2,
#endif
#if defined(SPI3)
        SPI3,
#endif
#if defined(SPI4)
        SPI4,
#endif
#if defined(SPI5)
        SPI5,
#endif
#if defined(SPI6)
        SPI6,
#endif
    };

    return spi_instance[id-1];
}

IRQn_Type _spi_irq_get(uint8_t id)
{
    IRQn_Type spi_irq_array[6] = {
        SPI1_IRQn,
#if defined(SPI2)
        SPI2_IRQn,
#endif
#if defined(SPI3)
        SPI3_IRQn,
#endif
#if defined(SPI4)
        SPI4_IRQn,
#endif
#if defined(SPI5)
        SPI5_IRQn,
#endif
#if defined(SPI6)
        SPI6_IRQn,
#endif
    };

    return spi_irq_array[id-1];
}

#if defined(DRV_STM32_H7)
void _spi_set_clocksrc(struct spi_dev_s *dev)
{
    struct up_spi_dev_s *priv = dev->priv;

    RCC_PeriphCLKInitTypeDef spi_clk_init = {0};

    uint32_t spi_clk[CONFIG_STM32_SPI_NUM] = {
        RCC_PERIPHCLK_SPI1, 
#if defined(SPI2)
        RCC_PERIPHCLK_SPI2,
#endif
#if defined(SPI3)
        RCC_PERIPHCLK_SPI3, 
#endif
#if defined(SPI4)
        RCC_PERIPHCLK_SPI4, 
#endif
#if defined(SPI5)
        RCC_PERIPHCLK_SPI5,
#endif
#if defined(SPI6)
        RCC_PERIPHCLK_SPI6,
#endif
    };

    uint32_t spi_clk_src[CONFIG_STM32_SPI_NUM] = {
        RCC_SPI1CLKSOURCE_PLL, 
#if defined(SPI2)
        RCC_SPI2CLKSOURCE_PLL,
#endif
#if defined(SPI3)
        RCC_SPI3CLKSOURCE_PLL, 
#endif
#if defined(SPI4)
        RCC_SPI4CLKSOURCE_D2PCLK1, 
#endif
#if defined(SPI5)
        RCC_SPI5CLKSOURCE_D2PCLK1,
#endif
#if defined(SPI6)
        RCC_SPI6CLKSOURCE_D3PCLK1,
#endif
    };

	spi_clk_init.PeriphClockSelection = spi_clk[priv->id-1];

    if (priv->id <= 3) {
        spi_clk_init.Spi123ClockSelection = spi_clk_src[priv->id-1];
    } else if (priv->id <= 5) {
        spi_clk_init.Spi45ClockSelection = spi_clk_src[priv->id-1];
    } else {
        spi_clk_init.Spi6ClockSelection = spi_clk_src[priv->id-1];
    }

	HAL_RCCEx_PeriphCLKConfig(&spi_clk_init);
}

uint32_t _spi_get_clock(struct spi_dev_s *dev)
{
    struct up_spi_dev_s *priv = dev->priv;
    uint32_t freq;

    uint64_t src[CONFIG_STM32_SPI_NUM] = {
        RCC_PERIPHCLK_SPI123, 
#if defined(SPI2)
        RCC_PERIPHCLK_SPI123,
#endif
#if defined(SPI3)
        RCC_PERIPHCLK_SPI123, 
#endif
#if defined(SPI4)
        RCC_PERIPHCLK_SPI45, 
#endif
#if defined(SPI5)
        RCC_PERIPHCLK_SPI45,
#endif
#if defined(SPI6)
        RCC_PERIPHCLK_SPI6,
#endif
    };
    freq = HAL_RCCEx_GetPeriphCLKFreq(src[priv->id-1]);

    return freq;
}

#endif

void _spi_pin_config(struct spi_dev_s *dev)
{
    struct up_spi_dev_s *priv = dev->priv;
    struct periph_pin_t *sckpin = &priv->sckpin;
    struct periph_pin_t *misopin = &priv->misopin;
    struct periph_pin_t *mosipin = &priv->mosipin;

#if defined (DRV_STM32_H7)
    LOW_PERIPH_INITPIN(sckpin->port, sckpin->pin, IOMODE_AFPP, IO_PULLUP, IO_SPEEDMAX, sckpin->alternate);
    LOW_PERIPH_INITPIN(misopin->port, misopin->pin, IOMODE_AFPP, IO_PULLUP, IO_SPEEDMAX, misopin->alternate);
    LOW_PERIPH_INITPIN(mosipin->port, mosipin->pin, IOMODE_AFPP, IO_PULLUP, IO_SPEEDMAX, mosipin->alternate);
#endif

#if defined (DRV_STM32_F1)
    LOW_PERIPH_INITPIN(sckpin->port, sckpin->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH);
    LOW_PERIPH_INITPIN(misopin->port, misopin->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH);
    LOW_PERIPH_INITPIN(mosipin->port, mosipin->pin, IOMODE_INPUT, IO_NOPULL, IO_SPEEDHIGH);
#endif

#if defined (DRV_STM32_F4)
    LOW_PERIPH_INITPIN(sckpin->port, sckpin->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, sckpin->alternate);
    LOW_PERIPH_INITPIN(misopin->port, misopin->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, misopin->alternate);
    LOW_PERIPH_INITPIN(mosipin->port, mosipin->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, mosipin->alternate);
#endif
}

void _spi_dma_setup(struct spi_dev_s *dev, uint8_t flag)
{
    int ret = 0;
    struct up_spi_dev_s *priv = dev->priv;
    struct dma_config *dmacfg = ((void *)0);
    DMA_HandleTypeDef *dmaobj = ((void *)0);

    if (!dmacfg->enable) {
        return;
    }

    if (DEVICE_SPI_DMA_RX == flag) {
        dmacfg = &priv->rxdma_cfg;
        dmaobj = &priv->rxdma;
        __HAL_LINKDMA(&priv->hspi, hdmarx, priv->rxdma);
    } else if (DEVICE_SPI_DMA_TX == flag) {
        dmacfg = &priv->txdma_cfg;
        dmaobj = &priv->txdma;
        __HAL_LINKDMA(&priv->hspi, hdmatx, priv->txdma);
    }

    {
        uint32_t tmpreg = 0x00U;
#if defined(DRV_STM32_F1)
        /* enable DMA clock && Delay after an RCC peripheral clock enabling*/
        SET_BIT(RCC->AHBENR, dmacfg->dma_rcc);

        tmpreg = READ_BIT(RCC->AHBENR, dmacfg->dma_rcc);
#elif defined(DRV_STM32_F4)|| defined(DRV_STM32_H7)
        SET_BIT(RCC->AHB1ENR, dmacfg->dma_rcc);

        /* Delay after an RCC peripheral clock enabling */
        tmpreg = READ_BIT(RCC->AHB1ENR, dmacfg->dma_rcc);
#endif
        UNUSED(tmpreg); /* To avoid compiler warnings */
    }

    dmaobj->Instance                 = dmacfg->instance;
#if defined(DRV_STM32_F4) 
    dmaobj->Init.Channel             = dmacfg->channel;
#elif defined(DRV_STM32_H7)
    dmaobj->Init.Request             = dmacfg->request;
#endif
    dmaobj->Init.PeriphInc            = DMA_PINC_DISABLE;
    dmaobj->Init.MemInc               = DMA_MINC_ENABLE;
    dmaobj->Init.PeriphDataAlignment  = DMA_PDATAALIGN_BYTE;
    dmaobj->Init.MemDataAlignment     = DMA_MDATAALIGN_BYTE;
    dmaobj->Init.Mode                 = DMA_NORMAL;

    if (DEVICE_SPI_DMA_RX == flag) {
        dmaobj->Init.Direction = DMA_PERIPH_TO_MEMORY;
        dmaobj->Init.Priority = DMA_PRIORITY_HIGH;
    } else if (DEVICE_SPI_DMA_TX == flag) {
        dmaobj->Init.Direction = DMA_MEMORY_TO_PERIPH;
        dmaobj->Init.Priority = DMA_PRIORITY_LOW;
    }

#if  defined(DRV_STM32_H7) || defined(DRV_STM32_F4) 
    dmaobj->Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    dmaobj->Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    dmaobj->Init.MemBurst            = DMA_MBURST_INC4;
    dmaobj->Init.PeriphBurst         = DMA_PBURST_INC4;
#endif

    if (HAL_DMA_DeInit(dmaobj) != HAL_OK) {
        ret = -1;
        return;
    }

    if (HAL_DMA_Init(dmaobj) != HAL_OK) {
        ret = -2;
        return;
    }

    HAL_NVIC_SetPriority(dmacfg->dma_irq, dmacfg->priority, 0);
    HAL_NVIC_EnableIRQ(dmacfg->dma_irq);
}

void _spi_setup(struct spi_dev_s *dev)
{
    int ret = 0;
    struct up_spi_dev_s *priv = dev->priv;

    priv->hspi.Instance               = _spi_obj_get(priv->id-1);
    priv->hspi.Init.Mode              = SPI_MODE_MASTER;
    priv->hspi.Init.Direction         = SPI_DIRECTION_2LINES;
    switch (dev->nbits) {
    case 8: priv->hspi.Init.DataSize = SPI_DATASIZE_8BIT; break;
    case 16: priv->hspi.Init.DataSize = SPI_DATASIZE_16BIT; break;
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

#if defined(APBPERIPH_BASE)
    priv->clock = HAL_RCC_GetPCLK1Freq();
#elif defined(APB1PERIPH_BASE) || defined(APB2PERIPH_BASE)
#if defined(DRV_STM32_H7)
    priv->clock = _spi_get_clock(dev);
#else
    if ((uint32_t)_spi_obj_get(priv->id-1) >= APB2PERIPH_BASE) {

        priv->clock = HAL_RCC_GetPCLK2Freq();
    } else {

        priv->clock = HAL_RCC_GetPCLK1Freq();
    }
#endif
#endif
    if (dev->frequency >= priv->clock / 2) {

        priv->hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    } else if (dev->frequency >= priv->clock / 4) {

        priv->hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    } else if (dev->frequency >= priv->clock / 8) {

        priv->hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    } else if (dev->frequency >= priv->clock / 16) {

        priv->hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    } else if (dev->frequency >= priv->clock / 32) {

        priv->hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    } else if (dev->frequency >= priv->clock / 64) {

        priv->hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    } else if (dev->frequency >= priv->clock / 128) {

        priv->hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
    } else {
        /*  min prescaler 256 */
        priv->hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    }

    priv->hspi.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    priv->hspi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    priv->hspi.State = HAL_SPI_STATE_RESET;
#if defined(DRV_STM32_H7)
    priv->hspi.Init.Mode                       = SPI_MODE_MASTER;
    priv->hspi.Init.NSS                        = SPI_NSS_SOFT;
    priv->hspi.Init.NSSPMode                   = SPI_NSS_PULSE_DISABLE;
    priv->hspi.Init.NSSPolarity                = SPI_NSS_POLARITY_LOW;
    priv->hspi.Init.CRCPolynomial              = 7;
    priv->hspi.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    priv->hspi.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    priv->hspi.Init.MasterSSIdleness           = SPI_MASTER_SS_IDLENESS_00CYCLE;
    priv->hspi.Init.MasterInterDataIdleness    = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    priv->hspi.Init.MasterReceiverAutoSusp     = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    priv->hspi.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
    priv->hspi.Init.IOSwap                     = SPI_IO_SWAP_DISABLE;
    priv->hspi.Init.FifoThreshold              = SPI_FIFO_THRESHOLD_01DATA;
#endif

    if (HAL_SPI_Init(&priv->hspi) != HAL_OK) {
        ret = -1;
    }
}

void _spi_irq(struct spi_dev_s *dev)
{
	struct up_spi_dev_s *priv = dev->priv;
	HAL_SPI_IRQHandler(&priv->hspi);
}

void _spi_irq_dmatx(struct spi_dev_s *dev)
{
	struct up_spi_dev_s *priv = dev->priv;
	dev->txresult = 0x01;
    spi_dmatxwakeup(dev);
}

void _spi_irq_dmarx(struct spi_dev_s *dev)
{
	struct up_spi_dev_s *priv = dev->priv;
    dev->rxresult = 0x01;
    spi_dmarxwakeup(dev);
}

void _spi_irq_dmartx(struct spi_dev_s *dev)
{
	struct up_spi_dev_s *priv = dev->priv;
    dev->txresult = 0x01;
	spi_dmatxwakeup(dev);
}

/****************************************************************************
 * Public Function Interface 
 ****************************************************************************/
uint32_t up_spi_setfrequency(struct spi_dev_s *dev, uint32_t frequency)
{
    struct up_spi_dev_s *priv = dev->priv;

    uint32_t setbits = 0;
    uint32_t actual  = 0;

#if defined (DRV_STM32_H7)
    const uint32_t mask = SPI_CFG1_MBR_Msk;
    const uint32_t cfg_FPCLKd2 = (0 << SPI_CFG1_MBR_Pos);
    const uint32_t cfg_FPCLKd4 = (1 << SPI_CFG1_MBR_Pos);
    const uint32_t cfg_FPCLKd8 = (2 << SPI_CFG1_MBR_Pos);
    const uint32_t cfg_FPCLKd16 = (3 << SPI_CFG1_MBR_Pos);
    const uint32_t cfg_FPCLKd32 = (4 << SPI_CFG1_MBR_Pos);
    const uint32_t cfg_FPCLKd64 = (5 << SPI_CFG1_MBR_Pos);
    const uint32_t cfg_FPCLKd128 = (6 << SPI_CFG1_MBR_Pos);
    const uint32_t cfg_FPCLKd256 = (7 << SPI_CFG1_MBR_Pos);
#elif defined (DRV_STM32_F1) || defined (DRV_STM32_F4)
    const uint32_t mask = SPI_CR1_BR_Msk;
    const uint32_t cfg_FPCLKd2 = (0 << SPI_CR1_BR_Pos);
    const uint32_t cfg_FPCLKd4 = (1 << SPI_CR1_BR_Pos);
    const uint32_t cfg_FPCLKd8 = (2 << SPI_CR1_BR_Pos);
    const uint32_t cfg_FPCLKd16 = (3 << SPI_CR1_BR_Pos);
    const uint32_t cfg_FPCLKd32 = (4 << SPI_CR1_BR_Pos);
    const uint32_t cfg_FPCLKd64 = (5 << SPI_CR1_BR_Pos);
    const uint32_t cfg_FPCLKd128 = (6 << SPI_CR1_BR_Pos);
    const uint32_t cfg_FPCLKd256 = (7 << SPI_CR1_BR_Pos);
#endif

    /* Limit to max possible (if STM32_SPI_CLK_MAX is defined in board.h) */

    if (frequency > priv->clock) {
        frequency = priv->clock;
    }

    /* Has the frequency changed? */

    if (frequency != dev->frequency) {

        /* Choices are limited by PCLK frequency with a set of divisors */

        if (frequency >= priv->clock >> 1) {

            /* More than fPCLK/2.  This is as fast as we can go */
            setbits = cfg_FPCLKd2; /* 000: fPCLK/2 */
            actual = priv->clock >> 1;
        } else if (frequency >= priv->clock >> 2) {

            /* Between fPCLCK/2 and fPCLCK/4, pick the slower */
            setbits = cfg_FPCLKd4; /* 001: fPCLK/4 */
            actual = priv->clock >> 2;
        } else if (frequency >= priv->clock >> 3) {

            /* Between fPCLCK/4 and fPCLCK/8, pick the slower */
            setbits = cfg_FPCLKd8; /* 010: fPCLK/8 */
            actual = priv->clock >> 3;
        } else if (frequency >= priv->clock >> 4) {

            /* Between fPCLCK/8 and fPCLCK/16, pick the slower */
            setbits = cfg_FPCLKd16; /* 011: fPCLK/16 */
            actual = priv->clock >> 4;
        } else if (frequency >= priv->clock >> 5) {

            /* Between fPCLCK/16 and fPCLCK/32, pick the slower */
            setbits = cfg_FPCLKd32; /* 100: fPCLK/32 */
            actual = priv->clock >> 5;
        } else if (frequency >= priv->clock >> 6) {

            /* Between fPCLCK/32 and fPCLCK/64, pick the slower */
            setbits = cfg_FPCLKd64; /*  101: fPCLK/64 */
            actual = priv->clock >> 6;
        } else if (frequency >= priv->clock >> 7) {

            /* Between fPCLCK/64 and fPCLCK/128, pick the slower */
            setbits = cfg_FPCLKd128; /* 110: fPCLK/128 */
            actual = priv->clock >> 7;
        } else {

            /* Less than fPCLK/128.  This is as slow as we can go */
            setbits = cfg_FPCLKd256; /* 111: fPCLK/256 */
            actual = priv->clock >> 8;
        }

		__HAL_SPI_DISABLE(&priv->hspi);
#if defined (DRV_STM32_H7)
        MODIFY_REG(priv->hspi.Instance->CFG1, mask, setbits);
#elif defined (DRV_STM32_F1) || defined (DRV_STM32_F4)
        MODIFY_REG(priv->hspi.Instance->CR1, mask, setbits);
#endif
        __HAL_SPI_ENABLE(&priv->hspi);

        dev->frequency = frequency;
        priv->actual_freq    = actual;
    }

    return priv->actual_freq;
}

void up_spi_setmode(struct spi_dev_s *dev, enum spi_mode_e mode)
{
    struct up_spi_dev_s *priv = dev->priv;

#if defined (DRV_STM32_H7)
    uint32_t setbits;
    uint32_t clrbits;
#endif

#if defined (DRV_STM32_F4) || defined (DRV_STM32_F1)
    uint16_t setbits;
    uint16_t clrbits;
#endif

	if (mode != dev->mode) {
#if defined (DRV_STM32_H7)

        switch (mode) {
        case SPIDEV_MODE0: /* CPOL=0; CPHA=0 */
            setbits = 0;
            clrbits = SPI_CFG2_CPOL | SPI_CFG2_CPHA;
            break;

        case SPIDEV_MODE1: /* CPOL=0; CPHA=1 */
            setbits = SPI_CFG2_CPHA;
            clrbits = SPI_CFG2_CPOL;
            break;

        case SPIDEV_MODE2: /* CPOL=1; CPHA=0 */
            setbits = SPI_CFG2_CPOL;
            clrbits = SPI_CFG2_CPHA;
            break;

        case SPIDEV_MODE3: /* CPOL=1; CPHA=1 */
            setbits = SPI_CFG2_CPOL | SPI_CFG2_CPHA;
            clrbits = 0;
            break;

        default:
            return;
        }

		__HAL_SPI_DISABLE(&priv->hspi);
		MODIFY_REG(priv->hspi.Instance->CFG2, clrbits, setbits);
        __HAL_SPI_ENABLE(&priv->hspi);

        while ((READ_REG(priv->hspi.Instance->SR) & SPI_SR_RXPLVL_0) != 0) {
            /* Flush SPI read FIFO */
            READ_REG(priv->hspi.Instance->RXDR);
        }
#endif

#if defined (DRV_STM32_F4) || defined (DRV_STM32_F1)
		switch (mode) {
			case SPIDEV_MODE0: /* CPOL=0; CPHA=0 */
			setbits = 0;
			clrbits = SPI_CR1_CPOL | SPI_CR1_CPHA;
			break;

		case SPIDEV_MODE1: /* CPOL=0; CPHA=1 */
			setbits = SPI_CR1_CPHA;
			clrbits = SPI_CR1_CPOL;
			break;

		case SPIDEV_MODE2: /* CPOL=1; CPHA=0 */
			setbits = SPI_CR1_CPOL;
			clrbits = SPI_CR1_CPHA;
			break;

		case SPIDEV_MODE3: /* CPOL=1; CPHA=1 */
			setbits = SPI_CR1_CPOL | SPI_CR1_CPHA;
			clrbits = 0;
			break;

		#ifdef SPI_CR2_FRF    /* If MCU supports TI Synchronous Serial Frame Format */
		case SPIDEV_MODETI:
			setbits = 0;
			clrbits = SPI_CR1_CPOL | SPI_CR1_CPHA;
			break;
		#endif

		default:
			return;
		}

		__HAL_SPI_DISABLE(&priv->hspi);
        MODIFY_REG(priv->hspi.Instance->CR1, clrbits, setbits);
		__HAL_SPI_ENABLE(&priv->hspi);

#ifdef SPI_CR2_FRF    /* If MCU supports TI Synchronous Serial Frame Format */
		switch (mode) {
		case SPIDEV_MODE0:
		case SPIDEV_MODE1:
		case SPIDEV_MODE2:
		case SPIDEV_MODE3:
            setbits = 0;
            clrbits = SPI_CR2_FRF;
            break;

		case SPIDEV_MODETI:
            setbits = SPI_CR2_FRF;
            clrbits = 0;
            break;

		default:
            return;
		}

		MODIFY_REG(priv->hspi.Instance->CR1, 0, SPI_CR1_SPE);
        MODIFY_REG(priv->hspi.Instance->CR1, clrbits, setbits);
		MODIFY_REG(priv->hspi.Instance->CR1, SPI_CR1_SPE, 0);
#endif

#endif
		dev->mode = mode;
	}
}

void up_spi_setbits(struct spi_dev_s *dev, int nbits)
{
    struct up_spi_dev_s *priv = dev->priv;

#if defined (DRV_STM32_H7)
    uint32_t setbits;
    uint32_t clrbits;
#endif

#if defined (DRV_STM32_F4) || defined (DRV_STM32_F1)
    uint16_t setbits;
    uint16_t clrbits;
#endif

    if (nbits != dev->nbits) {
#if defined (DRV_STM32_H7)
        if (nbits < 4 || nbits > 32) {
            return;
        }

#define SPI_CFG1_DSIZE_SHIFT      (0) /* Bits 0-4: number of bits in at single SPI data frame */
#define SPI_CFG1_DSIZE_VAL(n)     ((n-1) << SPI_CFG1_DSIZE_SHIFT)
#define SPI_CFG1_DSIZE_MASK       (0x1f << SPI_CFG1_DSIZE_SHIFT)

#define SPI_CFG1_FTHLV_SHIFT      (5) /* Bits 5-8: FIFO threshold level */
#define SPI_CFG1_FTHLV_MASK       (0xf << SPI_CFG1_FTHLV_SHIFT)

#define SPI_CFG1_FTHLV_1DATA    (0 << SPI_CFG1_FTHLV_SHIFT)
#define SPI_CFG1_FTHLV_2DATA    (1 << SPI_CFG1_FTHLV_SHIFT)

        clrbits = SPI_CFG1_DSIZE_MASK;
        setbits = SPI_CFG1_DSIZE_VAL(nbits);
		/* REVISIT: FIFO threshold level */

		/* If nbits is <=8, then we are in byte mode and FRXTH shall be set
		 * (else, transaction will not complete).
		 */

		if (nbits < 9) {
		    setbits |= SPI_CFG1_FTHLV_1DATA; /* RX FIFO Threshold = 1 byte */
		} else {
		    setbits |= SPI_CFG1_FTHLV_2DATA; /* RX FIFO Threshold = 2 byte */
		}

        __HAL_SPI_DISABLE(&priv->hspi);
		MODIFY_REG(priv->hspi.Instance->CFG1, clrbits, setbits);
        __HAL_SPI_ENABLE(&priv->hspi);
#endif

#if defined (DRV_STM32_F4) || defined (DRV_STM32_F1)
		switch (nbits) {
		case 8:
			priv->hspi.Init.DataSize = SPI_DATASIZE_8BIT;
			setbits = 0;
            clrbits = SPI_CR1_DFF;
			break;
		case 16:
			priv->hspi.Init.DataSize = SPI_DATASIZE_16BIT;
			setbits = SPI_CR1_DFF;
            clrbits = 0;
			break;
		default :break;
		}

		__HAL_SPI_DISABLE(&priv->hspi);
        MODIFY_REG(priv->hspi.Instance->CR1, clrbits, setbits);
		__HAL_SPI_ENABLE(&priv->hspi);
#endif
        dev->nbits = nbits;
	}
}

int up_spi_lock(struct spi_dev_s *dev, bool lock)
{
	return spi_devlock(dev, lock);
}

int up_spi_select(struct spi_dev_s *dev, uint32_t devid, bool selected)
{
    struct up_spi_dev_s *priv = dev->priv;
	int i = 0;
	for (; i < CONFIG_SPI_ATTACH_CS_NUM; i++) {
        if (devid == priv->dev_cs[i].id && priv->dev_cs[i].port != NULL) {
			HAL_GPIO_WritePin(priv->dev_cs[i].port, (0x01 << priv->dev_cs[i].pin), !selected);
			return GOK;
		}
	}
	return -1;
}

int up_spi_exchange(struct spi_dev_s *dev, const void *txbuffer, void *rxbuffer, size_t nwords)
{
    int ret = 0;
    struct up_spi_dev_s *priv = dev->priv;
    void *nc_txbuffer = (void *)txbuffer;

	// bug: when write sensor register data:
	// call HAL_SPI_TransmitReceive_DMA(dev, {addr, data}, {0x00, 0x00}, 2), it can not work
	// call HAL_SPI_Transmit_DMA(dev, {addr, data}, 2), it works
	// ???

	if (!nc_txbuffer && rxbuffer) {
		dev->rxresult = 0x00;
		if (HAL_OK != HAL_SPI_Receive_DMA(&priv->hspi, rxbuffer, nwords)) {
            return -1;
        }
        ret = spi_dmarxwait(dev);
	} else if (nc_txbuffer && !rxbuffer) {
		dev->txresult = 0x00;
		if (HAL_OK != HAL_SPI_Transmit_DMA(&priv->hspi, nc_txbuffer, nwords)) {
            return -1;
        }
        ret = spi_dmatxwait(dev);
	} else if (nc_txbuffer && rxbuffer) {
		dev->txresult = 0x00;
		if (HAL_OK != HAL_SPI_TransmitReceive_DMA(&priv->hspi, nc_txbuffer, rxbuffer, nwords)) {
            return -1;
        }
        ret = spi_dmatxwait(dev);
	}

	return ret;
}

int up_spi_exchangeblock(struct spi_dev_s *dev,
	const void *txbuffer, void *rxbuffer, size_t nwords)
{
	struct up_spi_dev_s *priv = dev->priv;
    void *nc_txbuffer = (void *)txbuffer;

    if (HAL_OK != HAL_SPI_TransmitReceive(&priv->hspi, nc_txbuffer, rxbuffer, nwords, 1000)) {
        return -1;
    }

	return GOK;
}

int up_spi_sndblock(struct spi_dev_s *dev, const void *buffer, size_t nwords)
{
	struct up_spi_dev_s *priv = dev->priv;
	void *nc_buffer = (void *)buffer;

    if (HAL_OK != HAL_SPI_Transmit(&priv->hspi, nc_buffer, nwords, 1000)) {
        return -1;
    }

	return GOK;
}

int up_spi_recvblock(struct spi_dev_s *dev, void *buffer, size_t nwords)
{
	int ret = 0;
	struct up_spi_dev_s *priv = dev->priv;

    if (HAL_OK != HAL_SPI_Receive(&priv->hspi, buffer, nwords, 1000)) {
        return -1;
    }

	return GOK;
}

int up_spi_setup(struct spi_dev_s *dev)
{
    struct up_spi_dev_s *priv = dev->priv;

#if defined(DRV_STM32_H7)
    _spi_set_clocksrc(dev);
#endif
    _spi_rcc_init(priv->id);

    _spi_pin_config(dev);

    _spi_setup(dev);

    _spi_dma_setup(dev, DEVICE_SPI_DMA_TX);
    _spi_dma_setup(dev, DEVICE_SPI_DMA_RX);

	HAL_NVIC_SetPriority(_spi_irq_get(priv->id), priv->priority, 0);
	HAL_NVIC_EnableIRQ(_spi_irq_get(priv->id));

    g_spi_list[priv->id-1] = dev;

    return GOK;
}

/****************************************************************************
 * STM32 HAL Callback 
 ****************************************************************************/

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    uint8_t idx = 0;
    idx = _spi_instance_judge(hspi);
    _spi_irq_dmartx(g_spi_list[idx]);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    uint8_t idx = 0;
    idx = _spi_instance_judge(hspi);
    _spi_irq_dmatx(g_spi_list[idx]);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    uint8_t idx = 0;
    idx = _spi_instance_judge(hspi);
    _spi_irq_dmarx(g_spi_list[idx]);
}

/****************************************************************************
 * STM32 IRQ
 ****************************************************************************/
void SPI1_IRQHandler(void)
{
	_spi_irq(g_spi_list[0]);
}

#if defined(SPI2)
void SPI2_IRQHandler(void)
{
	_spi_irq(g_spi_list[1]);
}
#endif

#if defined(SPI3)
void SPI3_IRQHandler(void)
{
	_spi_irq(g_spi_list[2]);
}
#endif

#if defined(SPI4)
void SPI4_IRQHandler(void)
{
	_spi_irq(g_spi_list[3]);
}
#endif

#if defined(SPI5)
void SPI5_IRQHandler(void)
{
	_spi_irq(g_spi_list[4]);
}
#endif

#if defined(SPI6)
void SPI6_IRQHandler(void)
{
	_spi_irq(g_spi_list[5]);
}
#endif
