#include <drv_qspi.h>

void _qspi_dma_setup(struct qspi_dev_s *dev)
{
    int ret = 0;
    struct up_qspi_dev_s *priv = dev->priv;
    struct dma_config *dmacfg = &priv->xferdma_cfg;
    DMA_HandleTypeDef *dmaobj = &priv->xferdma;

    if (!dmacfg->enable) {
        return;
    }

    __HAL_LINKDMA(&priv->hqspi, hdma, priv->xferdma);

    if (dmacfg->dma_rcc == RCC_AHB1ENR_DMA1EN) {
        __HAL_RCC_DMA1_CLK_ENABLE();
    } else {
        __HAL_RCC_DMA2_CLK_ENABLE();
    }

    HAL_DMA_DeInit(qspi_bus->QSPI_Handler.hdma);
    DMA_HandleTypeDef hdma_quadspi_config = QSPI_DMA_CONFIG;
    qspi_bus->hdma_quadspi = hdma_quadspi_config;

    if (HAL_DMA_Init(&qspi_bus->hdma_quadspi) != HAL_OK) {
        ret = -1
    }

    HAL_NVIC_SetPriority(dmacfg->dma_irq, 0, 0);
    HAL_NVIC_EnableIRQ(dmacfg->dma_irq);


}

void _qspi_setup(struct qspi_dev_s *dev)
{
    int ret = 0;
    struct up_qspi_dev_s *priv = dev->priv;

    priv->hqspi.Instance = QUADSPI;
    priv->hqspi.Init.FifoThreshold = 4;
    priv->hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
    priv->hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_4_CYCLE;
    priv->hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;  /* default mode */
    priv->hqspi.Init.ClockPrescaler = 9;             /* default clock prescaler 10 */
    priv->hqspi.Init.FlashSize = POSITION_VAL(dev->medium_size) - 1;

    ret = HAL_QSPI_Init(&priv->hqspi);
    if (ret != HAL_OK) {
        ret = -1;
    }
}

int up_qspi_setup(struct qspi_dev_s *dev)
{


    HAL_NVIC_SetPriority(QUADSPI_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(QUADSPI_IRQn);

}
static int stm32_qspi_init(struct rt_qspi_device *device, struct rt_qspi_configuration *qspi_cfg)
{
    int result = RT_EOK;
    unsigned int i = 1;

    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(qspi_cfg != RT_NULL);

    struct rt_spi_configuration *cfg = &qspi_cfg->parent;
    struct stm32_qspi_bus *qspi_bus = device->parent.bus->parent.user_data;
    rt_memset(&qspi_bus->QSPI_Handler, 0, sizeof(qspi_bus->QSPI_Handler));

    QSPI_HandleTypeDef QSPI_Handler_config = QSPI_BUS_CONFIG;
    qspi_bus->QSPI_Handler = QSPI_Handler_config;
    {                                                          \
        .Instance = QUADSPI,                                   \
        .Init.FifoThreshold = 4,                               \
        .Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE, \
        .Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_4_CYCLE,  \
    }


#if defined(SOC_SERIES_STM32MP1)
    while (cfg->max_hz < HAL_RCC_GetACLKFreq() / (i + 1))
#else
    while (cfg->max_hz < HAL_RCC_GetHCLKFreq() / (i + 1))
#endif
   {
        i++;
        if (i == 255)
        {
            LOG_E("QSPI init failed, QSPI frequency(%d) is too low.", cfg->max_hz);
            return -RT_ERROR;
        }
    }
    /* 80/(1+i) */
    qspi_bus->QSPI_Handler.Init.ClockPrescaler = i;

    if (!(cfg->mode & RT_SPI_CPOL))
    {
        /* QSPI MODE0 */
        qspi_bus->QSPI_Handler.Init.ClockMode = QSPI_CLOCK_MODE_0;
    }
    else
    {
        /* QSPI MODE3 */
        qspi_bus->QSPI_Handler.Init.ClockMode = QSPI_CLOCK_MODE_3;
    }

    /* flash size */
    qspi_bus->QSPI_Handler.Init.FlashSize = POSITION_VAL(qspi_cfg->medium_size) - 1;

    result = HAL_QSPI_Init(&qspi_bus->QSPI_Handler);
    if (result  == HAL_OK)
    {
        LOG_D("qspi init success!");
    }
    else
    {
        LOG_E("qspi init failed (%d)!", result);
    }

#ifdef BSP_QSPI_USING_DMA
    /* QSPI interrupts must be enabled when using the HAL_QSPI_Receive_DMA */
    HAL_NVIC_SetPriority(QSPI_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(QSPI_IRQn);
    HAL_NVIC_SetPriority(QSPI_DMA_IRQ, 0, 0);
    HAL_NVIC_EnableIRQ(QSPI_DMA_IRQ);

    /* init QSPI DMA */
    if(QSPI_DMA_RCC  == RCC_AHB1ENR_DMA1EN)
    {
        __HAL_RCC_DMA1_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_DMA2_CLK_ENABLE();
    }

    HAL_DMA_DeInit(qspi_bus->QSPI_Handler.hdma);
    DMA_HandleTypeDef hdma_quadspi_config = QSPI_DMA_CONFIG;
    qspi_bus->hdma_quadspi = hdma_quadspi_config;

    if (HAL_DMA_Init(&qspi_bus->hdma_quadspi) != HAL_OK)
    {
        LOG_E("qspi dma init failed (%d)!", result);
    }

    __HAL_LINKDMA(&qspi_bus->QSPI_Handler, hdma, qspi_bus->hdma_quadspi);
#endif /* BSP_QSPI_USING_DMA */

    return result;
}
