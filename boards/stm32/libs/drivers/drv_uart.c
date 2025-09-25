#include "drv_uart.h"

struct uart_dev_s *g_uart_list[CONFIG_STM32_UART_NUM] = {0};

static uint8_t           _uart_instance_judge(UART_HandleTypeDef *huart);
static void              _uart_rcc_init(uint8_t id);
static USART_TypeDef*    _uart_obj_get(uint8_t id);
static IRQn_Type         _uart_irq_get(uint8_t id);
static uint32_t          _uart_get_mask(uint32_t word_length, uint32_t parity);

static bool _usart_pinconfig(struct uart_dev_s *dev);
static void _usart_dma_setup(struct uart_dev_s *dev, uint8_t flag);
static void _usart_setup(struct uart_dev_s *dev);
static void _usart_irq(struct uart_dev_s *dev);
static void _usart_irq_dmatx(struct uart_dev_s *dev);
static void _usart_irq_dmarx(struct uart_dev_s *dev);

static int  up_usart_setup(struct uart_dev_s *dev);
static bool up_usart_txready(struct uart_dev_s *dev);
static bool up_usart_rxavailable(struct uart_dev_s *dev);
static int  up_usart_dmasend(struct uart_dev_s *dev, const uint8_t *p, uint16_t len);
static int  up_usart_send(struct uart_dev_s *dev, const uint8_t *p, uint16_t len);
static int  up_usart_readbuf(struct uart_dev_s *dev, uint8_t *p, uint16_t len);
static void up_usart_rxclear(struct uart_dev_s *dev);

const struct uart_ops_s g_uart_ops = 
{
    .setup = up_usart_setup,
    .txready = up_usart_txready,
    .rxavailable = up_usart_rxavailable,
    .dmasend = up_usart_dmasend,
    .send = up_usart_send,
    .readbuf = up_usart_readbuf,
    .rxclear = up_usart_rxclear,
};

/****************************************************************************
 * Private Function
 ****************************************************************************/
void _uart_rcc_init(uint8_t id)
{
	switch (id) {
        case 1:	__HAL_RCC_USART1_CLK_ENABLE();	break;
        case 2:	__HAL_RCC_USART2_CLK_ENABLE();	break;
#if defined(USART3)
        case 3: __HAL_RCC_USART3_CLK_ENABLE();  break;
#endif
#if defined(USART4)
        case 4: __HAL_RCC_UART4_CLK_ENABLE();  break;
#endif
#if defined(UART5)
        case 5: __HAL_RCC_UART5_CLK_ENABLE();  break;
#endif
#if defined(USART6)
        case 6: __HAL_RCC_USART6_CLK_ENABLE();  break;
#endif
#if defined(UART7)
        case 7: __HAL_RCC_UART7_CLK_ENABLE();  break;
#endif
#if defined(UART8)
        case 8: __HAL_RCC_UART8_CLK_ENABLE();  break;
#endif
    }
}

uint8_t _uart_instance_judge(UART_HandleTypeDef *huart)
{
    uint8_t idx;
    if      (huart->Instance == USART1) idx = 0;
    else if (huart->Instance == USART2) idx = 1;
#if defined(USART3)
    else if (huart->Instance == USART3) idx = 2;
#endif
#if defined(USART4)
    else if (huart->Instance == UART4)  idx = 3;
#endif
#if defined(UART5)
    else if (huart->Instance == UART5)  idx = 4;
#endif
#if defined(USART6)
    else if (huart->Instance == USART6) idx = 5;
#endif
#if defined(UART7)
    else if (huart->Instance == UART7)  idx = 6;
#endif
#if defined(UART8)
    else if (huart->Instance == UART8)  idx = 7;
#endif

    return idx;
}

USART_TypeDef* _uart_obj_get(uint8_t id)
{
    USART_TypeDef *uart_array[CONFIG_STM32_UART_NUM] = {
        USART1, 
        USART2, 
#if defined(USART3)
        USART3,
#endif
#if defined(UART4)
        UART4,
#endif
#if defined(UART5)
        UART5,
#endif
#if defined(USART6)
        USART6,
#endif
#if defined(UART7)
        UART7,
#endif
#if defined(UART8)
        UART8,
#endif
    };

    return uart_array[id-1];
}

IRQn_Type _uart_irq_get(uint8_t id)
{
	IRQn_Type uart_irq_array[CONFIG_STM32_UART_NUM] = {
        USART1_IRQn, 
        USART2_IRQn, 
#if defined(USART3)
        USART3_IRQn,
#endif
#if defined(UART4)
        UART4_IRQn,
#endif
#if defined(UART5)
        UART5_IRQn,
#endif
#if defined(USART6)
        USART6_IRQn,
#endif
#if defined(UART7)
        UART7_IRQn,
#endif
#if defined(UART8)
        UART8_IRQn,
#endif
    };

    return uart_irq_array[id-1];
}

uint32_t _uart_get_mask(uint32_t word_length, uint32_t parity)
{
    uint32_t mask = 0;
    if (word_length == UART_WORDLENGTH_8B) {
        if (parity == UART_PARITY_NONE) {
            mask = 0x00FFU ;
        } else {
            mask = 0x007FU ;
        }
    } else if (word_length == UART_WORDLENGTH_9B) {
        if (parity == UART_PARITY_NONE) {
            mask = 0x01FFU ;
        } else {
            mask = 0x00FFU ;
        }
#if defined (DRV_STM32_H7) && defined (DRV_STM32_WL)
    } else if (word_length == UART_WORDLENGTH_7B) {
        if (parity == UART_PARITY_NONE) {
            mask = 0x007FU ;
        } else {
            mask = 0x003FU ;
        }
#endif
    } else {
        mask = 0x0000U;
    }
    return mask;
}

bool _usart_pinconfig(struct uart_dev_s *dev)
{
    struct up_uart_dev_s *priv = dev->priv;
    struct periph_pin_t *txpin = &priv->txpin;
    struct periph_pin_t *rxpin = &priv->rxpin;

#if defined (DRV_STM32_F4) || defined(DRV_STM32_H7) || defined(DRV_STM32_WL)
    LOW_PERIPH_INITPIN(txpin->port, txpin->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, txpin->alternate);
    LOW_PERIPH_INITPIN(rxpin->port, rxpin->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, rxpin->alternate); 
#endif

#if defined (DRV_STM32_F1)
    LOW_PERIPH_INITPIN(txpin->port, txpin->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH);
    LOW_PERIPH_INITPIN(rxpin->port, rxpin->pin, IOMODE_INPUT,IO_NOPULL, IO_SPEEDHIGH); 
#endif

    return true;
}

void _usart_dma_setup(struct uart_dev_s *dev, uint8_t flag)
{
    int ret = 0;
    struct up_uart_dev_s *priv = dev->priv;
    struct dma_config *dmacfg = ((void *)0);
    DMA_HandleTypeDef *dmaobj = ((void *)0);

    if (!dmacfg->enable) {
        return;
    }

    if (DEVICE_USART_DMA_RX == flag) {
        dmacfg = &priv->rxdma_cfg;
        dmaobj = &priv->rxdma;
        __HAL_LINKDMA(&priv->com, hdmarx, priv->rxdma);
    } else if (DEVICE_USART_DMA_TX == flag) {
        dmacfg = &priv->txdma_cfg;
        dmaobj = &priv->txdma;
        __HAL_LINKDMA(&priv->com, hdmatx, priv->txdma);
    }

    {
        uint32_t tmpreg = 0x00U;
        /* enable DMA clock && Delay after an RCC peripheral clock enabling*/
#if defined(DRV_STM32_F1)
        SET_BIT(RCC->AHBENR, dmacfg->dma_rcc);
        tmpreg = READ_BIT(RCC->AHBENR, dmacfg->dma_rcc);
#elif defined(DRV_STM32_WL) || defined(DRV_STM32_H7) || defined(DRV_STM32_F4)
        SET_BIT(RCC->AHB1ENR, dmacfg->dma_rcc);
        tmpreg = READ_BIT(RCC->AHB1ENR, dmacfg->dma_rcc);
#endif

#if (defined(DRV_STM32_WL)) && defined(DMAMUX1)
        /* enable DMAMUX clock for L4+ and G4 */
        __HAL_RCC_DMAMUX1_CLK_ENABLE();
#endif

        UNUSED(tmpreg);
    }

#if defined(DRV_STM32_F1)
    dmaobj->Instance                 = dmacfg->instance;
#elif defined(DRV_STM32_F4) 
    dmaobj->Instance                 = dmacfg->instance;
    dmaobj->Init.Channel             = dmacfg->channel;
#elif defined(DRV_STM32_WL) || defined(DRV_STM32_H7)
    dmaobj->Instance                 = dmacfg->instance;
    dmaobj->Init.Request             = dmacfg->request;
#endif
    dmaobj->Init.PeriphInc            = DMA_PINC_DISABLE;
    dmaobj->Init.MemInc               = DMA_MINC_ENABLE;
    dmaobj->Init.PeriphDataAlignment  = DMA_PDATAALIGN_BYTE;
    dmaobj->Init.MemDataAlignment     = DMA_MDATAALIGN_BYTE;

    if (DEVICE_USART_DMA_RX == flag) {
        dmaobj->Init.Direction = DMA_PERIPH_TO_MEMORY;
        dmaobj->Init.Mode = DMA_NORMAL;
    } else if (DEVICE_USART_DMA_TX == flag) {
        dmaobj->Init.Direction = DMA_MEMORY_TO_PERIPH;
        dmaobj->Init.Mode = DMA_NORMAL;
    }

    dmaobj->Init.Priority = DMA_PRIORITY_MEDIUM;
#if  defined(DRV_STM32_H7)
    dmaobj->Init.FIFOMode = DMA_FIFOMODE_DISABLE;
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

    if (DEVICE_USART_DMA_RX == flag) {
        dev->dmarx.halflag = 0;
        HAL_UART_Receive_DMA(&priv->com, dev->dmarx.buffer, dev->dmarx.capacity/2);
    }
}

void _usart_setup(struct uart_dev_s *dev)
{
    struct up_uart_dev_s *priv = dev->priv;

    priv->com.Instance = _uart_obj_get(priv->id);
    priv->com.Init.BaudRate = dev->baudrate;

    switch (dev->wordlen) {
    case 7:
#if defined (DRV_STM32_H7) && defined (DRV_STM32_WL)
        priv->com.Init.WordLength = UART_WORDLENGTH_7B;
#endif
        break;
    case 8: priv->com.Init.WordLength = UART_WORDLENGTH_8B; break;
    case 9: priv->com.Init.WordLength = UART_WORDLENGTH_9B; break;
    }

    switch (dev->stopbitlen) {
    case 1: priv->com.Init.StopBits = UART_STOPBITS_1; break;     
    case 2: priv->com.Init.StopBits = UART_STOPBITS_2; break;
    }

    switch (dev->parity) {
    case 'n':
    case 'N': priv->com.Init.Parity = UART_PARITY_NONE; break;
    case 'o':
    case 'O': priv->com.Init.Parity = UART_PARITY_ODD; break;
    case 'e':
    case 'E': priv->com.Init.Parity = UART_PARITY_EVEN; break;     
    }

    priv->com.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    priv->com.Init.Mode = UART_MODE_TX_RX;
#if defined (DRV_STM32_WL)
    priv->com.Init.OverSampling = UART_OVERSAMPLING_16;
    priv->com.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    priv->com.Init.ClockPrescaler = UART_PRESCALER_DIV1;
#endif
    HAL_UART_Init(&priv->com);
#if defined (DRV_STM32_WL)
    HAL_UARTEx_DisableFifoMode(&priv->com);
#endif
}

void _usart_irq(struct uart_dev_s *dev)
{
    struct up_uart_dev_s *priv = dev->priv;
    __HAL_UART_DISABLE_IT(&priv->com, UART_IT_ERR);
    __HAL_UART_DISABLE_IT(&priv->com, UART_IT_PE);

    if ((__HAL_UART_GET_FLAG(&priv->com, UART_FLAG_RXNE) != RESET) &&
        (__HAL_UART_GET_IT_SOURCE(&priv->com, UART_IT_RXNE) != RESET)) {

        char chr = UART_GET_RDR(&priv->com, _uart_get_mask(priv->com.Init.WordLength, priv->com.Init.Parity));
        serial_buf_write(&dev->recv, (const uint8_t *)&chr, 1);
    } else if ((__HAL_UART_GET_FLAG(&priv->com, UART_FLAG_IDLE) != RESET) &&
        (__HAL_UART_GET_IT_SOURCE(&priv->com, UART_IT_IDLE) != RESET)) {

        _usart_irq_dmarx(dev);
        __HAL_UART_CLEAR_IDLEFLAG(&priv->com);
	}

	/* call hal for other uart irq */
	HAL_UART_IRQHandler(&priv->com); 
}

void _usart_irq_dmatx(struct uart_dev_s *dev)
{
    struct up_uart_dev_s *priv = dev->priv;
    uint32_t len = dev->xmit.size;

    if (len > 0) {
        if(len > dev->dmatx.capacity) {
            len = dev->dmatx.capacity;
        }
        serial_buf_read(&dev->xmit, &dev->dmatx.buffer[0], len);

#if defined (DRV_STM32_H7)
        /* save SRAM(DMA) data to D-Cache data */
        SCB_InvalidateDCache_by_Addr((uint32_t *)&dev->dmatx.buffer[0], dev->dmatx.capacity);
#endif

        HAL_UART_Transmit_DMA(&priv->com, &dev->dmatx.buffer[0], len);
    } else {
        serial_tx_post(dev);
    }
}

void _usart_irq_dmarx(struct uart_dev_s *dev)
{
    struct up_uart_dev_s *priv = dev->priv;
    uint32_t recv_len, counter;
    uint32_t dma_capacity = dev->dmarx.capacity/2;
    uint8_t *pre_buffer;
    uint8_t *next_buffer;
    uint8_t *rcv_buffer;
    uint32_t next_length;

    pre_buffer = (dev->dmarx.halflag == 0) ? dev->dmarx.buffer : dev->dmarx.buffer+dma_capacity;

    if (dev->dmarx.capacity == 0) {
        // config error or no use rxdma
        return;
    }

    HAL_UART_AbortReceive(&priv->com);

    counter = __HAL_DMA_GET_COUNTER(priv->com.hdmarx);
    recv_len = dma_capacity - counter;
    if (recv_len > dma_capacity) {
        recv_len = dma_capacity;
    }

    if (recv_len) {
        if (dev->dmarx.halflag == 0) {
            dev->dmarx.halflag = 1;
            rcv_buffer = dev->dmarx.buffer;
            next_buffer = dev->dmarx.buffer + dma_capacity;
        } else if (dev->dmarx.halflag == 1) {
            dev->dmarx.halflag = 0;
            rcv_buffer = dev->dmarx.buffer + dma_capacity;
            next_buffer = dev->dmarx.buffer;
        }
#if defined (DRV_STM32_H7)
        /* save SRAM(DMA) data to D-Cache data */
        SCB_InvalidateDCache_by_Addr((uint32_t *)&dev->dmarx.buffer[0], dev->dmarx.capacity);
#endif
        HAL_UART_Receive_DMA(&priv->com, next_buffer, dma_capacity);
        serial_buf_write(&dev->recv, rcv_buffer, recv_len);
    } else {
        HAL_UART_Receive_DMA(&priv->com, pre_buffer, dma_capacity);
    }
}

/****************************************************************************
 * Public Function Interface 
 ****************************************************************************/
int up_usart_setup(struct uart_dev_s *dev) 
{
    struct up_uart_dev_s *priv = dev->priv;

    _usart_pinconfig(dev);

	_uart_rcc_init(priv->id);

    _usart_setup(dev);

    _usart_dma_setup(dev, DEVICE_USART_DMA_TX);
    _usart_dma_setup(dev, DEVICE_USART_DMA_RX);

	HAL_NVIC_SetPriority(_uart_irq_get(priv->id), priv->priority, 0);
	HAL_NVIC_EnableIRQ(_uart_irq_get(priv->id));

    if (priv->rxdma_cfg.enable) {
        CLEAR_BIT(priv->com.Instance->CR3, USART_CR3_EIE);
        __HAL_UART_ENABLE_IT(&priv->com, UART_IT_IDLE);
    } else {
        CLEAR_BIT(priv->com.Instance->CR3, USART_CR3_EIE);
        __HAL_UART_ENABLE_IT(&priv->com, UART_IT_RXNE);
    }

    g_uart_list[priv->id - 1] = dev;

    return 0;
}

bool up_usart_txready(struct uart_dev_s *dev)
{
    return true;
}

bool up_usart_rxavailable(struct uart_dev_s *dev)
{
    return true;
}

int up_usart_dmasend(struct uart_dev_s *dev, const uint8_t *p, uint16_t len)
{
    struct up_uart_dev_s *priv = dev->priv;

    if (dev->dmatx.capacity == 0) {
        return -1;
    }

    if (serial_dev_lock(dev) != DTRUE) {
        return 1;
    }

    serial_buf_write(&dev->xmit, &p[0], len);

    if (serial_tx_wait(dev) != DTRUE) {
        serial_dev_unlock(dev);
        return 1;
    }

    uint16_t bufsize = (uint32_t)dev->xmit.size;
    bufsize = (bufsize <= dev->dmatx.capacity) ? bufsize : dev->dmatx.capacity;
    serial_buf_read(&dev->xmit, &dev->dmatx.buffer[0], bufsize);
#if defined (DRV_STM32_H7)
    /* save SRAM(DMA) data to D-Cache data */
    SCB_InvalidateDCache_by_Addr((uint32_t *)&dev->dmatx.buffer[0], dev->dmatx.capacity);
#endif
    HAL_UART_Transmit_DMA(&priv->com, &dev->dmatx.buffer[0], bufsize);

    serial_dev_unlock(dev);
    return len;
}

int up_usart_send(struct uart_dev_s *dev, const uint8_t *p, uint16_t len)
{
    int ret = 0;
    struct up_uart_dev_s *priv = dev->priv;

    if (serial_dev_lock(dev) != DTRUE) {
        return 1;
    }

    HAL_UART_Transmit(&priv->com, (uint8_t *)p, len, 3000);
    serial_dev_unlock(dev);

    return ret;
}

int up_usart_readbuf(struct uart_dev_s *dev, uint8_t *p, uint16_t len)
{
    int sz = 0;
    sz = serial_buf_read(&dev->recv, p, len);
    return sz;
}

void up_usart_rxclear(struct uart_dev_s *dev)
{
    serial_buf_clear(&dev->recv);
}

/****************************************************************************
 * STM32 HAL Callback 
 ****************************************************************************/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t idx = 0;
    idx = _uart_instance_judge(huart);
    _usart_irq_dmatx(g_uart_list[idx]);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t idx = 0;
    idx = _uart_instance_judge(huart);
    _usart_irq_dmarx(g_uart_list[idx]);
}

/****************************************************************************
 * STM32 Interrupt 
 ****************************************************************************/
void USART1_IRQHandler(void)
{
    _usart_irq(g_uart_list[0]);
}

void USART2_IRQHandler(void)
{	
    _usart_irq(g_uart_list[1]);
}

#if defined(USART3)
void USART3_IRQHandler(void)
{	
    _usart_irq(g_uart_list[2]);
}
#endif

#if defined(UART4)
void UART4_IRQHandler(void)
{	
    _usart_irq(g_uart_list[3]);
}
#endif

#if defined(UART5)
void UART5_IRQHandler(void)
{
    _usart_irq(g_uart_list[4]);
}
#endif

#if defined(USART6)
void USART6_IRQHandler(void)
{
    _usart_irq(g_uart_list[5]);
}
#endif

#if defined(UART7)
void UART7_IRQHandler(void)
{
    _usart_irq(g_uart_list[6]);
}
#endif

#if defined(UART8)
void UART8_IRQHandler(void)
{
    _usart_irq(g_uart_list[7]);
}
#endif

