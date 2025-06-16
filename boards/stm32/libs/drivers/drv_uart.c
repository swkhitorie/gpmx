#include "drv_uart.h"

struct uart_dev_s *g_uart_list[DRV_UART_PERIPHAL_NUM];

static bool low_pinconfig(struct uart_dev_s *dev);
static void low_dmatx_setup(struct uart_dev_s *dev);
static void low_dmarx_setup(struct uart_dev_s *dev);
static void low_setup(struct uart_dev_s *dev);
static void low_irq(struct uart_dev_s *dev);
static void low_irq_dmatx(struct uart_dev_s *dev);
static void low_irq_dmarx(struct uart_dev_s *dev);

static int  up_setup(struct uart_dev_s *dev);
static bool up_txready(struct uart_dev_s *dev);
static bool up_rxavailable(struct uart_dev_s *dev);
static int  up_dmasend(struct uart_dev_s *dev, const uint8_t *p, uint16_t len);
static int  up_send(struct uart_dev_s *dev, const uint8_t *p, uint16_t len);
static int  up_readbuf(struct uart_dev_s *dev, uint8_t *p, uint16_t len);
static void up_rxclear(struct uart_dev_s *dev);
const struct uart_ops_s g_uart_ops = 
{
    .setup = up_setup,
    .txready = up_txready,
    .rxavailable = up_rxavailable,
    .dmasend = up_dmasend,
    .send = up_send,
    .readbuf = up_readbuf,
    .rxclear = up_rxclear,
};

/****************************************************************************
 * Private Function
 ****************************************************************************/
bool low_pinconfig(struct uart_dev_s *dev)
{
    struct up_uart_dev_s *priv = dev->priv;
    uint8_t num = priv->id;
    uint8_t tx_selec = priv->pin_tx;
    uint8_t rx_selec = priv->pin_rx;

#if defined (DRV_BSP_H7)
	const struct pin_node *tx_pin;
	const struct pin_node *rx_pin;
	uint32_t illegal;

	switch (num) {
	case 1:
		if (PINNODE(uint32_t)(USART_PINCTRL_SOURCE(1, UART_PIN_TX, tx_selec)) != NULL && 
			PINNODE(uint32_t)(USART_PINCTRL_SOURCE(1, UART_PIN_RX, rx_selec)) != NULL) {
			tx_pin = USART_PINCTRL_SOURCE(1, UART_PIN_TX, tx_selec);
			rx_pin = USART_PINCTRL_SOURCE(1, UART_PIN_RX, rx_selec);
			illegal = tx_pin->port && rx_pin->port;
		}else {
			return false;
		}
		break;
	case 2:
		if (PINNODE(uint32_t)(USART_PINCTRL_SOURCE(2, UART_PIN_TX, tx_selec)) != NULL && 
			PINNODE(uint32_t)(USART_PINCTRL_SOURCE(2, UART_PIN_RX, rx_selec)) != NULL) {
			tx_pin = USART_PINCTRL_SOURCE(2, UART_PIN_TX, tx_selec);
			rx_pin = USART_PINCTRL_SOURCE(2, UART_PIN_RX, rx_selec);
			illegal = tx_pin->port && rx_pin->port;
		}else {
			return false;
		}
		break;
	case 3:
		if (PINNODE(uint32_t)(USART_PINCTRL_SOURCE(3, UART_PIN_TX, tx_selec)) != NULL && 
			PINNODE(uint32_t)(USART_PINCTRL_SOURCE(3, UART_PIN_RX, rx_selec)) != NULL) {
			tx_pin = USART_PINCTRL_SOURCE(3, UART_PIN_TX, tx_selec);
			rx_pin = USART_PINCTRL_SOURCE(3, UART_PIN_RX, rx_selec);
			illegal = tx_pin->port && rx_pin->port;
		}else {
			return false;
		}
		break;
	case 4:
		if (PINNODE(uint32_t)(UART_PINCTRL_SOURCE(4, UART_PIN_TX, tx_selec)) != NULL && 
			PINNODE(uint32_t)(UART_PINCTRL_SOURCE(4, UART_PIN_RX, rx_selec)) != NULL) {
			tx_pin = UART_PINCTRL_SOURCE(4, UART_PIN_TX, tx_selec);
			rx_pin = UART_PINCTRL_SOURCE(4, UART_PIN_RX, rx_selec);
			illegal = tx_pin->port && rx_pin->port;
		}else {
			return false;
		}
		break;
	case 5:
		if (PINNODE(uint32_t)(UART_PINCTRL_SOURCE(5, UART_PIN_TX, tx_selec)) != NULL && 
			PINNODE(uint32_t)(UART_PINCTRL_SOURCE(5, UART_PIN_RX, rx_selec)) != NULL) {
			tx_pin = UART_PINCTRL_SOURCE(5, UART_PIN_TX, tx_selec);
			rx_pin = UART_PINCTRL_SOURCE(5, UART_PIN_RX, rx_selec);
			illegal = tx_pin->port && rx_pin->port;
		}else {
			return false;
		}
		break;
	case 6:
		if (PINNODE(uint32_t)(USART_PINCTRL_SOURCE(6, UART_PIN_TX, tx_selec)) != NULL && 
			PINNODE(uint32_t)(USART_PINCTRL_SOURCE(6, UART_PIN_RX, rx_selec)) != NULL) {
			tx_pin = USART_PINCTRL_SOURCE(6, UART_PIN_TX, tx_selec);
			rx_pin = USART_PINCTRL_SOURCE(6, UART_PIN_RX, rx_selec);
			illegal = tx_pin->port && rx_pin->port;
		}else {
			return false;
		}
		break;
	case 7:
		if (PINNODE(uint32_t)(UART_PINCTRL_SOURCE(7, UART_PIN_TX, tx_selec)) != NULL && 
			PINNODE(uint32_t)(UART_PINCTRL_SOURCE(7, UART_PIN_RX, rx_selec)) != NULL) {
			tx_pin = UART_PINCTRL_SOURCE(7, UART_PIN_TX, tx_selec);
			rx_pin = UART_PINCTRL_SOURCE(7, UART_PIN_RX, rx_selec);
			illegal = tx_pin->port && rx_pin->port;
		}else {
			return false;
		}
		break;
	case 8:
		if (PINNODE(uint32_t)(UART_PINCTRL_SOURCE(8, UART_PIN_TX, tx_selec)) != NULL && 
			PINNODE(uint32_t)(UART_PINCTRL_SOURCE(8, UART_PIN_RX, rx_selec)) != NULL) {
			tx_pin = UART_PINCTRL_SOURCE(8, UART_PIN_TX, tx_selec);
			rx_pin = UART_PINCTRL_SOURCE(8, UART_PIN_RX, rx_selec);
			illegal = tx_pin->port && rx_pin->port;
		}else {
			return false;
		}
		break;
	default: return false;
	}

	if (illegal != 0) {
        low_gpio_setup(tx_pin->port, tx_pin->pin, IOMODE_AFPP, IO_PULLUP, IO_SPEEDHIGH, tx_pin->alternate, NULL, 0);
        low_gpio_setup(rx_pin->port, rx_pin->pin, IOMODE_AFPP, IO_PULLUP, IO_SPEEDHIGH, rx_pin->alternate, NULL, 0);    
	}else {
		return false;
	}
	return true;
#endif // End With Define DRV_BSP_H7

#if defined (DRV_BSP_F1)
    if (tx_selec == 0) {
        GPIO_TypeDef *tx_port[5] = { GPIOA,		GPIOA,      GPIOB,      GPIOC,      GPIOC};
        uint16_t       tx_pin[5] = {   9,         2,          10,        10,          12 };

        GPIO_TypeDef* rx_port[5] = { GPIOA,		GPIOA,      GPIOB,      GPIOC,      GPIOD};
        uint16_t       rx_pin[5] = {   10,        3,          11,        11,          2 };

        low_gpio_setup(tx_port[num-1], tx_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, 0, NULL, 0);
        low_gpio_setup(rx_port[num-1], rx_pin[num-1], IOMODE_INPUT,IO_NOPULL, IO_SPEEDHIGH, 0, NULL, 0); 
    } else if (tx_selec == 1 && num <= 3) {
        GPIO_TypeDef *tx_port[3] = { GPIOB,		GPIOD,      GPIOD };
        uint16_t       tx_pin[3] = {   6,         5,          8   };

        GPIO_TypeDef* rx_port[3] = { GPIOB,		GPIOD,      GPIOD };
        uint16_t       rx_pin[3] = {   7,        6,          9,   };

        low_gpio_setup(tx_port[num-1], tx_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, 0, NULL, 0);
        low_gpio_setup(rx_port[num-1], rx_pin[num-1], IOMODE_INPUT,IO_NOPULL, IO_SPEEDHIGH, 0, NULL, 0); 
    }
    return true;
#endif

#if defined (DRV_BSP_F4)
    if (tx_selec == 0) {
        GPIO_TypeDef *tx_port[8] = { GPIOA,		GPIOA,      GPIOB,      GPIOC,      GPIOC,  GPIOC,	GPIOF, GPIOE };
        uint16_t       tx_pin[8] = {   9,         2,          10,        10,          12,    6,   	7,      1    };

        GPIO_TypeDef* rx_port[8] = { GPIOA,		GPIOA,      GPIOB,      GPIOC,      GPIOD,  GPIOC,	GPIOF, GPIOE };
        uint16_t       rx_pin[8] = {   10,        3,          11,        11,          2,     7,		6,      0    };

        uint32_t      alternate[8] = {
            GPIO_AF7_USART1, GPIO_AF7_USART2, GPIO_AF7_USART3, GPIO_AF8_UART4,
            GPIO_AF8_UART5, GPIO_AF8_USART6, 
#if (BSP_CHIP_RESOURCE_LEVEL > 2)
            GPIO_AF8_UART7, GPIO_AF8_UART8
#endif
        };
        low_gpio_setup(tx_port[num-1], tx_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, alternate[num-1], NULL, 0);
        low_gpio_setup(rx_port[num-1], rx_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, alternate[num-1], NULL, 0); 
    } else if (tx_selec == 1) {
        GPIO_TypeDef *tx_port[8] = { GPIOB,		GPIOD,      GPIOD, GPIOD,   GPIOB,  GPIOG,  GPIOE,  /*GPIOJ*/};
        uint16_t       tx_pin[8] = {   6,         5,          8,     1,      13,     14,     8,     8    };

        GPIO_TypeDef* rx_port[8] = { GPIOB,		GPIOD,      GPIOD, GPIOD,   GPIOB,  GPIOG,  GPIOE,  /*GPIOJ*/};
        uint16_t       rx_pin[8] = {   7,        6,          9,      0,      12,     9,      7,     9    };

        uint32_t      alternate[3] = {
            GPIO_AF7_USART1, GPIO_AF7_USART2, GPIO_AF7_USART3, GPIO_AF8_UART4,
            GPIO_AF8_UART5, GPIO_AF8_USART6, 
#if (BSP_CHIP_RESOURCE_LEVEL > 2)
            GPIO_AF8_UART7, GPIO_AF8_UART8
#endif
        };

        low_gpio_setup(tx_port[num-1], tx_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, alternate[num-1], NULL, 0);
        low_gpio_setup(rx_port[num-1], rx_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, alternate[num-1], NULL, 0); 
    }
    return true;
#endif

#if defined (DRV_BSP_G0)
    return true;
#endif

#if defined (DRV_BSP_WL)
    if (tx_selec == 0) {
        GPIO_TypeDef *tx_port[8] = { GPIOA,		GPIOA   };
        uint16_t       tx_pin[8] = {   9,         2     };
        GPIO_TypeDef* rx_port[8] = { GPIOA,		GPIOA   };
        uint16_t       rx_pin[8] = {   10,        3     };
        uint32_t      alternate[8] = { GPIO_AF7_USART1, GPIO_AF7_USART2 };

        low_gpio_setup(tx_port[num-1], tx_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, alternate[num-1], NULL, 0);
        low_gpio_setup(rx_port[num-1], rx_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, alternate[num-1], NULL, 0);

    } else if (tx_selec == 1 && num == 1) {
        low_gpio_setup(GPIOB, 6, IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, GPIO_AF7_USART1, NULL, 0);
        low_gpio_setup(GPIOB, 7, IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, GPIO_AF7_USART1, NULL, 0);
    }
    return true;
#endif
}

void low_dmatx_setup(struct uart_dev_s *dev)
{
    struct up_uart_dev_s *priv = dev->priv;
    uint8_t num = priv->id;
#if defined (DRV_BSP_H7)
	DMA_Stream_TypeDef *uart_txdma_stream[DRV_UART_PERIPHAL_NUM] = {
        DMA1_Stream0, DMA1_Stream1, DMA1_Stream2, DMA1_Stream3,
        DMA1_Stream4, DMA1_Stream5, DMA1_Stream6, DMA1_Stream7
    };
	uint8_t uart_txdma_request[DRV_UART_PERIPHAL_NUM] = {
        DMA_REQUEST_USART1_TX, DMA_REQUEST_USART2_TX, DMA_REQUEST_USART3_TX, 
		DMA_REQUEST_UART4_TX, DMA_REQUEST_UART5_TX, DMA_REQUEST_USART6_TX, 
        DMA_REQUEST_UART7_TX, DMA_REQUEST_UART8_TX
    };
    IRQn_Type uart_txdma_irq[DRV_UART_PERIPHAL_NUM] = {
        DMA1_Stream0_IRQn, DMA1_Stream1_IRQn, DMA1_Stream2_IRQn, DMA1_Stream3_IRQn,
        DMA1_Stream4_IRQn, DMA1_Stream5_IRQn, DMA1_Stream6_IRQn, DMA1_Stream7_IRQn
    };
#elif defined(DRV_BSP_F1)
    DMA_Channel_TypeDef *uart_txdma_channel[DRV_UART_PERIPHAL_NUM] = {
        DMA1_Channel4, DMA1_Channel7, DMA1_Channel2,
    };
    IRQn_Type uart_txdma_irq[DRV_UART_PERIPHAL_NUM] = {
        DMA1_Channel4_IRQn, DMA1_Channel7_IRQn, DMA1_Channel2_IRQn,
    };
#elif defined(DRV_BSP_F4)
	DMA_Stream_TypeDef *uart_txdma_stream[DRV_UART_PERIPHAL_NUM] = {
        DMA2_Stream7, DMA1_Stream6, DMA1_Stream3, DMA1_Stream4,
        DMA1_Stream7, DMA2_Stream6, DMA1_Stream1, DMA1_Stream0
    };
    uint32_t uart_txdma_channel[DRV_UART_PERIPHAL_NUM] = {
        DMA_CHANNEL_4, DMA_CHANNEL_4, DMA_CHANNEL_4, DMA_CHANNEL_4,
        DMA_CHANNEL_4, DMA_CHANNEL_5, DMA_CHANNEL_5, DMA_CHANNEL_5
    };
    IRQn_Type uart_txdma_irq[DRV_UART_PERIPHAL_NUM] = {
        DMA2_Stream7_IRQn, DMA1_Stream6_IRQn, DMA1_Stream3_IRQn, DMA1_Stream4_IRQn,
        DMA1_Stream7_IRQn, DMA2_Stream6_IRQn, DMA1_Stream1_IRQn, DMA1_Stream0_IRQn
    };
#elif defined(DRV_BSP_WL)
    DMA_Channel_TypeDef *uart_txdma_channel[DRV_UART_PERIPHAL_NUM] = {
        DMA1_Channel1, DMA1_Channel2
    };
    uint8_t uart_txdma_request[DRV_UART_PERIPHAL_NUM] = {
        DMA_REQUEST_USART1_TX, DMA_REQUEST_USART2_TX
    };
    IRQn_Type uart_txdma_irq[DRV_UART_PERIPHAL_NUM] = {
        DMA1_Channel1_IRQn, DMA1_Channel2_IRQn
    };
#endif  // End With Define DRV_BSP_F1, DRV_BSP_F4, DRV_BSP_H7, DRV_BSP_WL

    __HAL_LINKDMA(&priv->com, hdmatx, priv->txdma);
#if defined (DRV_BSP_H7)
    __HAL_RCC_DMA1_CLK_ENABLE();
    //HAL_DMA_DeInit(&obj->txdma);
    priv->txdma.Instance = uart_txdma_stream[num-1];
    priv->txdma.Init.Request = uart_txdma_request[num-1];
    priv->txdma.Init.Direction = DMA_MEMORY_TO_PERIPH;
    priv->txdma.Init.PeriphInc = DMA_PINC_DISABLE;
    priv->txdma.Init.MemInc = DMA_MINC_ENABLE;
    priv->txdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    priv->txdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    priv->txdma.Init.Mode = DMA_NORMAL;
    priv->txdma.Init.Priority = DMA_PRIORITY_MEDIUM;
    priv->txdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    priv->txdma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    priv->txdma.Init.MemBurst = DMA_MBURST_SINGLE;
    priv->txdma.Init.PeriphBurst = DMA_PBURST_SINGLE;
#elif defined (DRV_BSP_F1)
    __HAL_RCC_DMA1_CLK_ENABLE();
    priv->txdma.Instance = uart_txdma_channel[num-1];
    priv->txdma.Init.Direction = DMA_MEMORY_TO_PERIPH;
    priv->txdma.Init.PeriphInc = DMA_PINC_DISABLE;
    priv->txdma.Init.MemInc = DMA_MINC_ENABLE;
    priv->txdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    priv->txdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    priv->txdma.Init.Mode = DMA_NORMAL;
    priv->txdma.Init.Priority = DMA_PRIORITY_MEDIUM;
#elif defined (DRV_BSP_F4)
    if (num == 1 || num == 6) {
        __HAL_RCC_DMA2_CLK_ENABLE();
    } else {
        __HAL_RCC_DMA1_CLK_ENABLE();
    }
    priv->txdma.Instance = uart_txdma_stream[num-1];
    priv->txdma.Init.Channel = uart_txdma_channel[num-1];
    priv->txdma.Init.Direction = DMA_MEMORY_TO_PERIPH;
    priv->txdma.Init.PeriphInc = DMA_PINC_DISABLE;
    priv->txdma.Init.MemInc = DMA_MINC_ENABLE;
    priv->txdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    priv->txdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    priv->txdma.Init.Mode = DMA_NORMAL;
    priv->txdma.Init.Priority = DMA_PRIORITY_MEDIUM;
    priv->txdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
#elif defined (DRV_BSP_WL)
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();
    priv->txdma.Instance = uart_txdma_channel[num-1];
    priv->txdma.Init.Request = uart_txdma_request[num-1];
    priv->txdma.Init.Direction = DMA_MEMORY_TO_PERIPH;
    priv->txdma.Init.PeriphInc = DMA_PINC_DISABLE;
    priv->txdma.Init.MemInc = DMA_MINC_ENABLE;
    priv->txdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    priv->txdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    priv->txdma.Init.Mode = DMA_NORMAL;
    priv->txdma.Init.Priority = DMA_PRIORITY_LOW;
#endif   // End With Define DRV_BSP_F1 DRV_BSP_F4 DRV_BSP_H7
    HAL_DMA_Init(&priv->txdma);
#if defined (DRV_BSP_WL) && defined (CORE_CM0PLUS)
    HAL_DMA_ConfigChannelAttributes(&priv->rxdma, DMA_CHANNEL_NPRIV);
#endif

    HAL_NVIC_SetPriority(uart_txdma_irq[num-1], priv->priority_dmatx, 0);
    HAL_NVIC_EnableIRQ(uart_txdma_irq[num-1]);
}

void low_dmarx_setup(struct uart_dev_s *dev)
{
    struct up_uart_dev_s *priv = dev->priv;
    uint8_t num = priv->id;
#if defined (DRV_BSP_H7)
	DMA_Stream_TypeDef *uart_rxdma_stream[DRV_UART_PERIPHAL_NUM] = {
        DMA2_Stream0, DMA2_Stream1, DMA2_Stream2, DMA2_Stream3,
        DMA2_Stream4, DMA2_Stream5, DMA2_Stream6, DMA2_Stream7
    };
	uint8_t uart_rxdma_request[DRV_UART_PERIPHAL_NUM] = {
        DMA_REQUEST_USART1_RX, DMA_REQUEST_USART2_RX, DMA_REQUEST_USART3_RX, 
		DMA_REQUEST_UART4_RX, DMA_REQUEST_UART5_RX, DMA_REQUEST_USART6_RX, 
        DMA_REQUEST_UART7_RX, DMA_REQUEST_UART8_RX
    };	
    IRQn_Type uart_rxdma_irq[DRV_UART_PERIPHAL_NUM] = {
        DMA2_Stream0_IRQn, DMA2_Stream1_IRQn, DMA2_Stream2_IRQn, DMA2_Stream3_IRQn,
        DMA2_Stream4_IRQn, DMA2_Stream5_IRQn, DMA2_Stream6_IRQn, DMA2_Stream7_IRQn
    };
#elif defined(DRV_BSP_F1)
    DMA_Channel_TypeDef *uart_rxdma_channel[DRV_UART_PERIPHAL_NUM] = {
        DMA1_Channel5, DMA1_Channel6, DMA1_Channel3,
    };
    IRQn_Type uart_rxdma_irq[DRV_UART_PERIPHAL_NUM] = {
        DMA1_Channel5_IRQn, DMA1_Channel6_IRQn, DMA1_Channel3_IRQn,
    };
#elif defined(DRV_BSP_F4)
	DMA_Stream_TypeDef *uart_rxdma_stream[DRV_UART_PERIPHAL_NUM] = {
        DMA2_Stream2, DMA1_Stream5, DMA1_Stream1, DMA1_Stream2,
        DMA1_Stream0, DMA2_Stream1, DMA1_Stream3, DMA1_Stream6
    };
    uint32_t uart_rxdma_channel[DRV_UART_PERIPHAL_NUM] = {
        DMA_CHANNEL_4, DMA_CHANNEL_4, DMA_CHANNEL_4, DMA_CHANNEL_4,
        DMA_CHANNEL_4, DMA_CHANNEL_5, DMA_CHANNEL_5, DMA_CHANNEL_5
    };
    IRQn_Type uart_rxdma_irq[DRV_UART_PERIPHAL_NUM] = {
        DMA2_Stream2_IRQn, DMA1_Stream5_IRQn, DMA1_Stream1_IRQn, DMA1_Stream2_IRQn,
        DMA1_Stream0_IRQn, DMA2_Stream1_IRQn, DMA1_Stream3_IRQn, DMA1_Stream6_IRQn
    };
#elif defined(DRV_BSP_WL)
    DMA_Channel_TypeDef *uart_rxdma_channel[DRV_UART_PERIPHAL_NUM] = {
        DMA2_Channel1, DMA2_Channel2
    };
    uint8_t uart_rxdma_request[DRV_UART_PERIPHAL_NUM] = {
        DMA_REQUEST_USART1_RX, DMA_REQUEST_USART2_RX
    };
    IRQn_Type uart_rxdma_irq[DRV_UART_PERIPHAL_NUM] = {
        DMA2_Channel1_IRQn, DMA2_Channel2_IRQn
    };
#endif  // End With Define DRV_BSP_F1, DRV_BSP_F4, DRV_BSP_H7

    __HAL_LINKDMA(&priv->com, hdmarx, priv->rxdma);
#if defined (DRV_BSP_H7)
#ifdef DMA2
    __HAL_RCC_DMA2_CLK_ENABLE();
#endif
    //HAL_DMA_DeInit(&obj->rxdma);
    priv->rxdma.Instance = uart_rxdma_stream[num-1];
    priv->rxdma.Init.Request = uart_rxdma_request[num-1];
    priv->rxdma.Init.Direction = DMA_PERIPH_TO_MEMORY;
    priv->rxdma.Init.PeriphInc = DMA_PINC_DISABLE;
    priv->rxdma.Init.MemInc = DMA_MINC_ENABLE;
    priv->rxdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    priv->rxdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    priv->rxdma.Init.Mode = DMA_NORMAL;
    priv->rxdma.Init.Priority = DMA_PRIORITY_MEDIUM;
    priv->rxdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    priv->rxdma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    priv->rxdma.Init.MemBurst = DMA_MBURST_SINGLE;
    priv->rxdma.Init.PeriphBurst = DMA_PBURST_SINGLE;
#elif defined (DRV_BSP_F1)
#ifdef DMA2
    __HAL_RCC_DMA2_CLK_ENABLE();
#endif
    priv->rxdma.Instance = uart_rxdma_channel[num-1];
    priv->rxdma.Init.Direction = DMA_PERIPH_TO_MEMORY;
    priv->rxdma.Init.PeriphInc = DMA_PINC_DISABLE;
    priv->rxdma.Init.MemInc = DMA_MINC_ENABLE;
    priv->rxdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    priv->rxdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    priv->rxdma.Init.Mode = DMA_NORMAL;
    priv->rxdma.Init.Priority = DMA_PRIORITY_MEDIUM;
#elif defined (DRV_BSP_F4)
    if (num == 1 || num == 6) {
        __HAL_RCC_DMA2_CLK_ENABLE();
    } else {
        __HAL_RCC_DMA1_CLK_ENABLE();
    }
    priv->rxdma.Instance = uart_rxdma_stream[num-1];
    priv->rxdma.Init.Channel = uart_rxdma_channel[num-1];
    priv->rxdma.Init.Direction = DMA_PERIPH_TO_MEMORY;
    priv->rxdma.Init.PeriphInc = DMA_PINC_DISABLE;
    priv->rxdma.Init.MemInc = DMA_MINC_ENABLE;
    priv->rxdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    priv->rxdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    priv->rxdma.Init.Mode = DMA_NORMAL;
    priv->rxdma.Init.Priority = DMA_PRIORITY_MEDIUM;
    priv->rxdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
#elif defined (DRV_BSP_WL)
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
    priv->rxdma.Instance = uart_rxdma_channel[num-1];;
    priv->rxdma.Init.Request = uart_rxdma_request[num-1];;
    priv->rxdma.Init.Direction = DMA_PERIPH_TO_MEMORY;
    priv->rxdma.Init.PeriphInc = DMA_PINC_DISABLE;
    priv->rxdma.Init.MemInc = DMA_MINC_ENABLE;
    priv->rxdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    priv->rxdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    priv->rxdma.Init.Mode = DMA_NORMAL;
    priv->rxdma.Init.Priority = DMA_PRIORITY_LOW;
#endif   // End With Define DRV_BSP_F4 and DRV_BSP_H7
    HAL_DMA_Init(&priv->rxdma);
#if defined (DRV_BSP_WL) && defined (CORE_CM0PLUS)
    HAL_DMA_ConfigChannelAttributes(&priv->rxdma, DMA_CHANNEL_NPRIV);
#endif

    HAL_NVIC_SetPriority(uart_rxdma_irq[num-1], priv->priority_dmarx, 0);
    HAL_NVIC_EnableIRQ(uart_rxdma_irq[num-1]);

    dev->dmarx.halflag = 0;
    HAL_UART_Receive_DMA(&priv->com, dev->dmarx.buffer, dev->dmarx.capacity/2);
}

void low_setup(struct uart_dev_s *dev)
{
    struct up_uart_dev_s *priv = dev->priv;
    uint8_t num = priv->id;
    USART_TypeDef *uart_array[DRV_UART_PERIPHAL_NUM] = {
        USART1, USART2, 
#if (BSP_CHIP_RESOURCE_LEVEL > 0)
        USART3, 
#if (BSP_CHIP_RESOURCE_LEVEL > 1)
        UART4, UART5,  
#if (BSP_CHIP_RESOURCE_LEVEL > 2)
        USART6, UART7,  UART8
#endif
#endif
#endif // End With Define BSP_CHIP_RESOURCE_LEVEL
    };
	IRQn_Type uart_irq_array[DRV_UART_PERIPHAL_NUM] = {
        USART1_IRQn, USART2_IRQn, 
#if (BSP_CHIP_RESOURCE_LEVEL > 0)
        USART3_IRQn, 
#if (BSP_CHIP_RESOURCE_LEVEL > 1)
        UART4_IRQn, 
        UART5_IRQn,
#if (BSP_CHIP_RESOURCE_LEVEL > 2)
        USART6_IRQn, UART7_IRQn, UART8_IRQn
#endif
#endif
#endif // End With Define BSP_CHIP_RESOURCE_LEVEL
    };

    low_pinconfig(dev);

	switch (num) {
        case 1:	__HAL_RCC_USART1_CLK_ENABLE();	break;
        case 2:	__HAL_RCC_USART2_CLK_ENABLE();	break;
    #if (BSP_CHIP_RESOURCE_LEVEL > 0)
        case 3:	__HAL_RCC_USART3_CLK_ENABLE();	break;
    #if (BSP_CHIP_RESOURCE_LEVEL > 1)
        case 4:	__HAL_RCC_UART4_CLK_ENABLE();	break;
        case 5:	__HAL_RCC_UART5_CLK_ENABLE();	break;
        case 6:	__HAL_RCC_USART6_CLK_ENABLE();	break;
    #if (BSP_CHIP_RESOURCE_LEVEL > 2)
        case 7:	__HAL_RCC_UART7_CLK_ENABLE();	break;
        case 8:	__HAL_RCC_UART8_CLK_ENABLE();	break;
    #endif
    #endif
    #endif // End With Define BSP_CHIP_RESOURCE_LEVEL
    }

    priv->com.Instance = uart_array[num-1];
    priv->com.Init.BaudRate = dev->baudrate;
    switch (dev->wordlen) {
    case 7:
#if !defined (DRV_BSP_F1) && !defined (DRV_BSP_F4) && !defined (DRV_BSP_WL)
        priv->com.Init.WordLength = UART_WORDLENGTH_7B;
#endif
        break;
    case 8:
        priv->com.Init.WordLength = UART_WORDLENGTH_8B;
        break;
    case 9:
        priv->com.Init.WordLength = UART_WORDLENGTH_9B;
        break;
    }
    switch (dev->stopbitlen) {
    case 1:
        priv->com.Init.StopBits = UART_STOPBITS_1;
        break;     
    case 2:
        priv->com.Init.StopBits = UART_STOPBITS_2;
        break;
    }
    switch (dev->parity) {
    case 'n':
    case 'N':
        priv->com.Init.Parity = UART_PARITY_NONE;
        break;
    case 'o':
    case 'O':
        priv->com.Init.Parity = UART_PARITY_ODD;
        break;
    case 'e':
    case 'E':
        priv->com.Init.Parity = UART_PARITY_EVEN;     
        break;     
    }
    priv->com.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    priv->com.Init.Mode = UART_MODE_TX_RX;
#if defined (DRV_BSP_WL)
    priv->com.Init.OverSampling = UART_OVERSAMPLING_16;
    priv->com.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    priv->com.Init.ClockPrescaler = UART_PRESCALER_DIV1;
#endif
    HAL_UART_Init(&priv->com);
#if defined (DRV_BSP_WL)
    HAL_UARTEx_SetTxFifoThreshold(&priv->com, UART_TXFIFO_THRESHOLD_1_8);
    HAL_UARTEx_SetRxFifoThreshold(&priv->com, UART_RXFIFO_THRESHOLD_1_8);
    HAL_UARTEx_DisableFifoMode(&priv->com);
#endif
    if (priv->enable_dmarx) {
        low_dmarx_setup(dev);
    }
    if (priv->enable_dmatx) {
        low_dmatx_setup(dev);
    }
	HAL_NVIC_SetPriority(uart_irq_array[num-1], priv->priority, 0);
	HAL_NVIC_EnableIRQ(uart_irq_array[num-1]);

	__HAL_UART_ENABLE_IT(&priv->com, UART_IT_IDLE);
    dev->tx_ready = true;

    g_uart_list[num-1] = dev;
}

void low_irq(struct uart_dev_s *dev)
{
    struct up_uart_dev_s *priv = dev->priv;
	__HAL_UART_DISABLE_IT(&priv->com, UART_IT_ERR);
	__HAL_UART_DISABLE_IT(&priv->com, UART_IT_PE);

	if ((__HAL_UART_GET_FLAG(&priv->com, UART_FLAG_IDLE) != RESET)) {
		__HAL_UART_CLEAR_IDLEFLAG(&priv->com);
#if defined (DRV_BSP_G0) || defined (DRV_BSP_H7) || defined (DRV_BSP_WL)
		uint32_t tmp = priv->com.Instance->ISR;
		tmp = priv->com.Instance->RDR;
#else
		uint32_t tmp = priv->com.Instance->SR;
		tmp = priv->com.Instance->DR;
#endif  // End With Define DRV_BSP_G0 and DRV_BSP_H7 and DRV_BSP_WL
        if (dev->dmarx.capacity != 0) {
            HAL_UART_AbortReceive(&priv->com);
            dev->rx_available = false;
            uint16_t rxlen = dev->dmarx.capacity/2 - __HAL_DMA_GET_COUNTER(priv->com.hdmarx);
#if defined (DRV_BSP_H7)
            /* save SRAM(DMA) data to D-Cache data */
            SCB_InvalidateDCache_by_Addr((uint32_t *)&dev->dmarx.buffer[0], dev->dmarx.capacity);
#endif
            if (dev->dmarx.halflag == 0) {
                HAL_UART_Receive_DMA(&priv->com, dev->dmarx.buffer + dev->dmarx.capacity/2, dev->dmarx.capacity/2);
                dev->dmarx.halflag = 1;
                uart_buf_write(&dev->recv, &dev->dmarx.buffer[0], rxlen);
            } else if (dev->dmarx.halflag == 1) {
                HAL_UART_Receive_DMA(&priv->com, dev->dmarx.buffer, dev->dmarx.capacity/2);
                dev->dmarx.halflag = 0;
                uart_buf_write(&dev->recv, dev->dmarx.buffer + dev->dmarx.capacity/2, rxlen);
            }
            dev->rx_available = true;
        }
	}
	
	/* Call HAL function for other UART IRQ (such as TC IRQ and RXNE IRQ) */
	HAL_UART_IRQHandler(&priv->com); 
}

void low_irq_dmatx(struct uart_dev_s *dev)
{
    struct up_uart_dev_s *priv = dev->priv;
    uint32_t len = dev->xmit.size;
#if defined(CONFIG_BOARD_FREERTOS_ENABLE) && defined(CONFIG_SERIAL_TASKSYNC)
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
#endif

    if (len > 0) {
        if(len > dev->dmatx.capacity) 
            len = dev->dmatx.capacity;
        uart_buf_read(&dev->xmit, &dev->dmatx.buffer[0], len);
        dev->tx_ready = false;
#if defined (DRV_BSP_H7)
            /* save SRAM(DMA) data to D-Cache data */
            SCB_InvalidateDCache_by_Addr((uint32_t *)&dev->dmatx.buffer[0], dev->dmatx.capacity);
#endif
        HAL_UART_Transmit_DMA(&priv->com, &dev->dmatx.buffer[0], len);
    } else {
        dev->tx_ready = true;
#if defined(CONFIG_BOARD_FREERTOS_ENABLE) && defined(CONFIG_SERIAL_TASKSYNC)
        xSemaphoreGiveFromISR(dev->xmitsem, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#endif
    }
}

void low_irq_dmarx(struct uart_dev_s *dev)
{
    struct up_uart_dev_s *priv = dev->priv;
    HAL_UART_AbortReceive(&priv->com);
    dev->rx_available = false;
#if defined (DRV_BSP_H7)
    /* save SRAM(DMA) data to D-Cache data */
    SCB_InvalidateDCache_by_Addr((uint32_t *)&dev->dmarx.buffer[0], dev->dmarx.capacity);
#endif
    if (dev->dmarx.halflag == 0) {
        HAL_UART_Receive_DMA(&priv->com, dev->dmarx.buffer + dev->dmarx.capacity/2, dev->dmarx.capacity/2);
        dev->dmarx.halflag = 1;
        uart_buf_write(&dev->recv, &dev->dmarx.buffer[0], dev->dmarx.capacity/2);
    } else if (dev->dmarx.halflag == 1) {
        HAL_UART_Receive_DMA(&priv->com, dev->dmarx.buffer, dev->dmarx.capacity/2);
        dev->dmarx.halflag = 0;
        uart_buf_write(&dev->recv, dev->dmarx.buffer + dev->dmarx.capacity/2, dev->dmarx.capacity/2);
    }
    dev->rx_available = true;
}

/****************************************************************************
 * Public Function Interface 
 ****************************************************************************/
int up_setup(struct uart_dev_s *dev) 
{
    low_setup(dev);
#if defined(CONFIG_BOARD_FREERTOS_ENABLE) && defined(CONFIG_SERIAL_TASKSYNC)
    dev->xmitsem = xSemaphoreCreateBinary();
    dev->recvsem = xSemaphoreCreateBinary();
    xSemaphoreGive(dev->xmitsem);
    xSemaphoreGive(dev->recvsem);
#endif
}

bool up_txready(struct uart_dev_s *dev)
{
    return dev->tx_ready;
}

bool up_rxavailable(struct uart_dev_s *dev)
{
    return dev->rx_available;
}

int up_dmasend(struct uart_dev_s *dev, const uint8_t *p, uint16_t len)
{
    struct up_uart_dev_s *priv = dev->priv;

#if defined(CONFIG_BOARD_FREERTOS_ENABLE) && defined(CONFIG_SERIAL_TASKSYNC)
    if (xSemaphoreTake(dev->xmitsem, 5000) != pdTRUE) {
        return 0;
    }
#endif

    uart_buf_write(&dev->xmit, &p[0], len);
    if (!dev->tx_ready) return false;
    dev->tx_ready = false;

    if (dev->dmatx.capacity != 0) {
        uint16_t bufsize = (uint32_t)dev->xmit.size;
        bufsize = (bufsize <= dev->dmatx.capacity) ? bufsize : dev->dmatx.capacity;
        uart_buf_read(&dev->xmit, &dev->dmatx.buffer[0], bufsize);
    #if defined (DRV_BSP_H7)
        /* save SRAM(DMA) data to D-Cache data */
        SCB_InvalidateDCache_by_Addr((uint32_t *)&dev->dmatx.buffer[0], dev->dmatx.capacity);
    #endif
        HAL_UART_Transmit_DMA(&priv->com, &dev->dmatx.buffer[0], bufsize);
    } else {
        uint16_t bufsize = (uint32_t)dev->xmit.size;
        // bug
        HAL_UART_Transmit_IT(&priv->com, &dev->xmit.buffer[0], bufsize);
    }
    return len;
}

int up_send(struct uart_dev_s *dev, const uint8_t *p, uint16_t len)
{
    struct up_uart_dev_s *priv = dev->priv;
#if defined(CONFIG_BOARD_FREERTOS_ENABLE) && defined(CONFIG_SERIAL_TASKSYNC)
    if (xSemaphoreTake(dev->xmitsem, 5000) != pdTRUE) {
        return 0;
    }
#endif
    HAL_UART_Transmit(&priv->com, (uint8_t *)p, len, 3000);
#if defined(CONFIG_BOARD_FREERTOS_ENABLE) && defined(CONFIG_SERIAL_TASKSYNC)
    xSemaphoreGive(dev->xmitsem);
#endif
    return len;
}

int up_readbuf(struct uart_dev_s *dev, uint8_t *p, uint16_t len)
{
    int sz = 0;
#if defined(CONFIG_BOARD_FREERTOS_ENABLE) && defined(CONFIG_SERIAL_TASKSYNC)
    if (xSemaphoreTake(dev->recvsem, 5000) != pdTRUE) {
        return 0;
    }
#endif
    sz = uart_buf_read(&dev->recv, p, len);
#if defined(CONFIG_BOARD_FREERTOS_ENABLE) && defined(CONFIG_SERIAL_TASKSYNC)
    xSemaphoreGive(dev->recvsem);
#endif
    return sz;
}

void up_rxclear(struct uart_dev_s *dev)
{
    uart_buf_clear(&dev->recv);
}

/****************************************************************************
 * STM32 HAL Library Callback 
 ****************************************************************************/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t idx = 0;
    if (huart->Instance == USART1)		idx = 0;
    else if (huart->Instance == USART2)	idx = 1;
#if (BSP_CHIP_RESOURCE_LEVEL > 0)
    else if (huart->Instance == USART3)	idx = 2;
#if (BSP_CHIP_RESOURCE_LEVEL > 1)
    else if (huart->Instance == UART4)	idx = 3;
    else if (huart->Instance == UART5)	idx = 4;
#if (BSP_CHIP_RESOURCE_LEVEL > 2)
    else if (huart->Instance == USART6)	idx = 5;
    else if (huart->Instance == UART7)	idx = 6;
    else if (huart->Instance == UART8)	idx = 7;
#endif
#endif
#endif

    low_irq_dmatx(g_uart_list[idx]);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t idx = 0;
    if (huart->Instance == USART1)		idx = 0;
    else if (huart->Instance == USART2)	idx = 1;
#if (BSP_CHIP_RESOURCE_LEVEL > 0)
    else if (huart->Instance == USART3)	idx = 2;
#if (BSP_CHIP_RESOURCE_LEVEL > 1)
    else if (huart->Instance == UART4)	idx = 3;
    else if (huart->Instance == UART5)	idx = 4;
#if (BSP_CHIP_RESOURCE_LEVEL > 2)
    else if (huart->Instance == USART6)	idx = 5;
    else if (huart->Instance == UART7)	idx = 6;
    else if (huart->Instance == UART8)	idx = 7;
#endif
#endif
#endif

    low_irq_dmarx(g_uart_list[idx]);
}

void USART1_IRQHandler(void)
{
    low_irq(g_uart_list[0]);
}

void USART2_IRQHandler(void)
{	
    low_irq(g_uart_list[1]);
}

#if (BSP_CHIP_RESOURCE_LEVEL > 0)
void USART3_IRQHandler(void)
{	
    low_irq(g_uart_list[2]);
}
#endif

#if (BSP_CHIP_RESOURCE_LEVEL > 1)

void UART4_IRQHandler(void)
{	
    low_irq(g_uart_list[3]);
}

void UART5_IRQHandler(void)
{
    low_irq(g_uart_list[4]);
}

void USART6_IRQHandler(void)
{
    low_irq(g_uart_list[5]);
}

void UART7_IRQHandler(void)
{
    low_irq(g_uart_list[6]);
}

void UART8_IRQHandler(void)
{
    low_irq(g_uart_list[7]);
}
#endif  // End With Define BSP_CHIP_RESOURCE_LEVEL
