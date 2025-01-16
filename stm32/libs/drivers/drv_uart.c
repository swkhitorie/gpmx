#include "drv_uart.h"

struct drv_uart_t *drv_uart_list[DRV_UART_PERIPHAL_NUM] = {
    0, 0, 0, 0, 0, 0, 0, 0
};

bool drv_uart_pinconfig(uint8_t num, uint8_t tx_selec, uint8_t rx_selec)
{
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
        drv_gpio_init(tx_pin->port, tx_pin->pin, IOMODE_AFPP,
						IO_PULLUP, IO_SPEEDHIGH, tx_pin->alternate, NULL);
        drv_gpio_init(rx_pin->port, rx_pin->pin, IOMODE_AFPP,
						IO_PULLUP, IO_SPEEDHIGH, rx_pin->alternate, NULL);    
	}else {
		return false;
	}
	return true;
#endif // End With Define DRV_BSP_H7

#if defined (DRV_BSP_G0) || defined (DRV_BSP_F1)
    if (tx_selec == 0) {
        GPIO_TypeDef *tx_port[5] = { GPIOA,		GPIOA,      GPIOB,      GPIOC,      GPIOC};
        uint16_t       tx_pin[5] = {   9,         2,          10,        10,          12 };

        GPIO_TypeDef* rx_port[5] = { GPIOA,		GPIOA,      GPIOB,      GPIOC,      GPIOD};
        uint16_t       rx_pin[5] = {   10,        3,          11,        11,          2 };

        drv_gpio_init(tx_port[num-1], tx_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, NULL);
        drv_gpio_init(rx_port[num-1], rx_pin[num-1], IOMODE_INPUT,IO_NOPULL, IO_SPEEDHIGH, NULL); 
    } else if (tx_selec == 1 && num <= 3) {
        GPIO_TypeDef *tx_port[3] = { GPIOB,		GPIOD,      GPIOD };
        uint16_t       tx_pin[3] = {   6,         5,          8   };

        GPIO_TypeDef* rx_port[3] = { GPIOB,		GPIOD,      GPIOD };
        uint16_t       rx_pin[3] = {   7,        6,          9,   };

        drv_gpio_init(tx_port[num-1], tx_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, NULL);
        drv_gpio_init(rx_port[num-1], rx_pin[num-1], IOMODE_INPUT,IO_NOPULL, IO_SPEEDHIGH, NULL); 
    }
#endif

}

void drv_uart_dma_attr_init(struct drv_uart_dma_attr_t *obj, 
                            uint8_t *p, uint32_t len, 
                            uint8_t priority)
{
    obj->mem_buff = p;
    obj->mem_capacity = len;
    obj->priority = priority;
}

void drv_uart_attr_init(struct drv_uart_attr_t *obj, 
                        uint32_t baudrate, uint32_t wordlen,
                        uint32_t stopbitlen, uint32_t parity,
                        uint8_t priority)
{
    obj->baudrate = baudrate;
    obj->wordlen = wordlen;
    obj->stopbitlen = stopbitlen;
    obj->parity = parity;
    obj->priority = priority;
}

void drv_uart_buff_init(struct drv_uart_t *obj, uint8_t *txbuf, uint16_t tlen,
                        uint8_t *rxbuf, uint16_t rlen)
{
    devbuf_init(&obj->tx_buf, txbuf, tlen);
    devbuf_init(&obj->rx_buf, rxbuf, rlen);
}

void drv_uart_init(uint8_t num, struct drv_uart_t *obj,
                    uint8_t tx_selc, uint8_t rx_selc,
                    struct drv_uart_attr_t *com_attr,
                    struct drv_uart_dma_attr_t *txdma_attr,
                    struct drv_uart_dma_attr_t *rxdma_attr)
{
    USART_TypeDef *uart_array[DRV_UART_PERIPHAL_NUM] = {
        USART1, USART2, USART3, 
#if (BSP_CHIP_RESOURCE_LEVEL > 1)
        UART4, UART5,  USART6, UART7,  UART8
#endif // End With Define BSP_CHIP_RESOURCE_LEVEL
    };
	IRQn_Type uart_irq_array[DRV_UART_PERIPHAL_NUM] = {
        USART1_IRQn, USART2_IRQn, USART3_IRQn, 
#if (BSP_CHIP_RESOURCE_LEVEL > 1)
        UART4_IRQn, 
        UART5_IRQn, USART6_IRQn, UART7_IRQn, UART8_IRQn
#endif // End With Define BSP_CHIP_RESOURCE_LEVEL
    };
#if defined (DRV_BSP_F4) || defined (DRV_BSP_H7)
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
#else
    DMA_Channel_TypeDef *uart_txdma_channel[DRV_UART_PERIPHAL_NUM] = {
        DMA1_Channel4, DMA1_Channel7, DMA1_Channel2,
    };
    IRQn_Type uart_txdma_irq[DRV_UART_PERIPHAL_NUM] = {
        DMA1_Channel4_IRQn, DMA1_Channel7_IRQn, DMA1_Channel2_IRQn,
    };
    DMA_Channel_TypeDef *uart_rxdma_channel[DRV_UART_PERIPHAL_NUM] = {
        DMA1_Channel5, DMA1_Channel6, DMA1_Channel3,
    };
    IRQn_Type uart_rxdma_irq[DRV_UART_PERIPHAL_NUM] = {
        DMA1_Channel5_IRQn, DMA1_Channel6_IRQn, DMA1_Channel3_IRQn,
    };
#endif  // End With Define DRV_BSP_F4 and DRV_BSP_H7

    obj->attr = *com_attr;
    drv_uart_pinconfig(num, tx_selc, rx_selc);

	switch (num) {
	case 1:	__HAL_RCC_USART1_CLK_ENABLE();	break;
	case 2:	__HAL_RCC_USART2_CLK_ENABLE();	break;
	case 3:	__HAL_RCC_USART3_CLK_ENABLE();	break;
#if (BSP_CHIP_RESOURCE_LEVEL > 1)
	case 4:	__HAL_RCC_UART4_CLK_ENABLE();	break;
	case 5:	__HAL_RCC_UART5_CLK_ENABLE();	break;
	case 6:	__HAL_RCC_USART6_CLK_ENABLE();	break;
	case 7:	__HAL_RCC_UART7_CLK_ENABLE();	break;
	case 8:	__HAL_RCC_UART8_CLK_ENABLE();	break;
#endif // End With Define BSP_CHIP_RESOURCE_LEVEL
	}

	obj->com.Instance = uart_array[num-1];
	obj->com.Init.BaudRate = obj->attr.baudrate;
    obj->com.Init.WordLength = obj->attr.wordlen;
    obj->com.Init.StopBits = obj->attr.stopbitlen;
    obj->com.Init.Parity = obj->attr.parity;
	obj->com.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	obj->com.Init.Mode = UART_MODE_TX_RX;
	HAL_UART_Init(&obj->com);

    if (txdma_attr != NULL) {
        obj->attr_txdma = *txdma_attr;
        __HAL_RCC_DMA1_CLK_ENABLE();
        __HAL_LINKDMA(&obj->com, hdmatx, obj->txdma);
#if defined (DRV_BSP_F4) || defined (DRV_BSP_H7)
        HAL_DMA_DeInit(&obj->txdma);
        obj->txdma.Instance = uart_txdma_stream[num-1];
        obj->txdma.Init.Request = uart_txdma_request[num-1];
        obj->txdma.Init.Direction = DMA_MEMORY_TO_PERIPH;
        obj->txdma.Init.PeriphInc = DMA_PINC_DISABLE;
        obj->txdma.Init.MemInc = DMA_MINC_ENABLE;
        obj->txdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        obj->txdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        obj->txdma.Init.Mode = DMA_NORMAL;
        obj->txdma.Init.Priority = DMA_PRIORITY_MEDIUM;
        obj->txdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        obj->txdma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
        obj->txdma.Init.MemBurst = DMA_MBURST_SINGLE;
        obj->txdma.Init.PeriphBurst = DMA_PBURST_SINGLE;
#else
        obj->txdma.Instance = uart_txdma_channel[num-1];
        obj->txdma.Init.Direction = DMA_MEMORY_TO_PERIPH;
        obj->txdma.Init.PeriphInc = DMA_PINC_DISABLE;
        obj->txdma.Init.MemInc = DMA_MINC_ENABLE;
        obj->txdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        obj->txdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        obj->txdma.Init.Mode = DMA_NORMAL;
        obj->txdma.Init.Priority = DMA_PRIORITY_MEDIUM;
#endif   // End With Define DRV_BSP_F4 and DRV_BSP_H7
        HAL_DMA_Init(&obj->txdma);
        HAL_NVIC_SetPriority(uart_txdma_irq[num-1], txdma_attr->priority, 0);
        HAL_NVIC_EnableIRQ(uart_txdma_irq[num-1]);
    } else {
        obj->attr_txdma.mem_capacity = 0;
    }

    if (rxdma_attr != NULL) {
        obj->attr_rxdma = *rxdma_attr;
#ifdef DMA2
        __HAL_RCC_DMA2_CLK_ENABLE();
#endif
        __HAL_LINKDMA(&obj->com, hdmarx, obj->rxdma);
#if defined (DRV_BSP_F4) || defined (DRV_BSP_H7)
        HAL_DMA_DeInit(&obj->rxdma);
        obj->rxdma.Instance = uart_rxdma_stream[num-1];
        obj->rxdma.Init.Request = uart_rxdma_request[num-1];
        obj->rxdma.Init.Direction = DMA_PERIPH_TO_MEMORY;
        obj->rxdma.Init.PeriphInc = DMA_PINC_DISABLE;
        obj->rxdma.Init.MemInc = DMA_MINC_ENABLE;
        obj->rxdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        obj->rxdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        obj->rxdma.Init.Mode = DMA_NORMAL;
        obj->rxdma.Init.Priority = DMA_PRIORITY_MEDIUM;
        obj->rxdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        obj->rxdma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
        obj->rxdma.Init.MemBurst = DMA_MBURST_SINGLE;
        obj->rxdma.Init.PeriphBurst = DMA_PBURST_SINGLE;
#else
        obj->rxdma.Instance = uart_rxdma_channel[num-1];
        obj->rxdma.Init.Direction = DMA_PERIPH_TO_MEMORY;
        obj->rxdma.Init.PeriphInc = DMA_PINC_DISABLE;
        obj->rxdma.Init.MemInc = DMA_MINC_ENABLE;
        obj->rxdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        obj->rxdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        obj->rxdma.Init.Mode = DMA_NORMAL;
        obj->rxdma.Init.Priority = DMA_PRIORITY_MEDIUM;
#endif   // End With Define DRV_BSP_F4 and DRV_BSP_H7
        HAL_DMA_Init(&obj->rxdma);
        HAL_NVIC_SetPriority(uart_rxdma_irq[num-1], rxdma_attr->priority, 0);
        HAL_NVIC_EnableIRQ(uart_rxdma_irq[num-1]);

        HAL_UART_Receive_DMA(&obj->com, obj->attr_rxdma.mem_buff, obj->attr_rxdma.mem_capacity);
    } else {
        obj->attr_rxdma.mem_capacity = 0;
    }

	HAL_NVIC_SetPriority(uart_irq_array[num-1], obj->attr.priority, 0);
	HAL_NVIC_EnableIRQ(uart_irq_array[num-1]);
	__HAL_UART_ENABLE_IT(&obj->com, UART_IT_IDLE);
	obj->tx_busy = false;

    drv_uart_list[num-1] = obj;
}

int drv_uart_send(struct drv_uart_t *obj, const uint8_t *p, uint16_t len, enum __drv_rwway way)
{
    switch (way) {
    case RWPOLL:
        HAL_UART_Transmit(&obj->com, (uint8_t *)p, len, 3000);
        break;
    case RWIT:
    case RWDMA:
        {
            devbuf_write(&obj->tx_buf, &p[0], len);
            if (obj->tx_busy) return false;
            obj->tx_busy = true;
            if (obj->attr_txdma.mem_capacity != 0) {
                uint16_t bufsize = (uint32_t)devbuf_size(&obj->tx_buf);
                bufsize = (bufsize <= obj->attr_txdma.mem_capacity) ? bufsize : 
                                obj->attr_txdma.mem_capacity;
                devbuf_read(&obj->tx_buf, &obj->attr_txdma.mem_buff[0], bufsize);
#if defined (DRV_BSP_H7)
                SCB_CleanDCache_by_Addr((uint32_t *)&obj->attr_txdma.mem_buff[0],
                                    obj->attr_txdma.mem_capacity);
#endif  // End With Define DRV_BSP_H7
                HAL_UART_Transmit_DMA(&obj->com, &obj->attr_txdma.mem_buff[0], bufsize);
            } else {
                uint16_t bufsize = (uint32_t)devbuf_size(&obj->tx_buf);
                // bug
                HAL_UART_Transmit_IT(&obj->com, &obj->tx_buf.buf[0], bufsize);
            }
            break; 
        }
    default: false;
    }
    return true;
}

devbuf_t drv_uart_devbuf(struct drv_uart_t *obj)
{
    return obj->rx_buf;
}

void drv_uart_irq(struct drv_uart_t *obj)
{
    if (obj == NULL) return;
	__HAL_UART_DISABLE_IT(&obj->com, UART_IT_ERR);
	__HAL_UART_DISABLE_IT(&obj->com, UART_IT_PE);

	if ((__HAL_UART_GET_FLAG(&obj->com, UART_FLAG_IDLE) != RESET)) {
		__HAL_UART_CLEAR_IDLEFLAG(&obj->com);
#if defined (DRV_BSP_G0) || defined (DRV_BSP_H7)
		uint32_t tmp = obj->com.Instance->ISR;
		tmp = obj->com.Instance->RDR;
#else
		uint32_t tmp = obj->com.Instance->SR;
		tmp = obj->com.Instance->DR;
#endif  // End With Define DRV_BSP_G0 and DRV_BSP_H7
		HAL_UART_AbortReceive(&obj->com);
        
		uint16_t rxlen = obj->attr_rxdma.mem_capacity - __HAL_DMA_GET_COUNTER(obj->com.hdmarx);

#if defined (DRV_BSP_H7)
		/* save SRAM(DMA) data to D-Cache data */
		SCB_InvalidateDCache_by_Addr((uint32_t *)&obj->attr_rxdma.mem_buff[0], 
                                obj->attr_rxdma.mem_capacity);
#endif  // End With Define DRV_BSP_H7
        devbuf_write(&obj->rx_buf, &obj->attr_rxdma.mem_buff[0], rxlen);
		HAL_UART_Receive_DMA(&obj->com, &obj->attr_rxdma.mem_buff[0], 
                                obj->attr_rxdma.mem_capacity);
	}
	
	/* Call HAL function for other UART IRQ (such as TC IRQ and RXNE IRQ) */
	HAL_UART_IRQHandler(&obj->com); 
}

void drv_uart_txdma_irq(struct drv_uart_t *obj)
{
    if (obj == NULL || obj->attr_txdma.mem_capacity == 0) return;
    uint32_t len = devbuf_size(&obj->tx_buf);

    if (len > 0) {
        if(len > obj->attr_txdma.mem_capacity) 
            len = obj->attr_txdma.mem_capacity;
        devbuf_read(&obj->tx_buf, &obj->attr_txdma.mem_buff[0], len);
        obj->tx_busy = true;

#if defined (DRV_BSP_H7)
        /* save SRAM(DMA) data to D-Cache data */
        SCB_InvalidateDCache_by_Addr((uint32_t *)&obj->attr_txdma.mem_buff[0],
                                    obj->attr_rxdma.mem_capacity);
#endif  // End With Define DRV_BSP_H7

        HAL_UART_Transmit_DMA(&obj->com, &obj->attr_txdma.mem_buff[0], len);
    }else {
        obj->tx_busy = false; 
    }
}

void drv_uart_rxdma_irq(struct drv_uart_t *obj)
{
    if (obj == NULL || obj->attr_rxdma.mem_capacity == 0) return;
    HAL_UART_AbortReceive(&obj->com);

#if defined (DRV_BSP_H7)
    /* save SRAM(DMA) data to D-Cache data */
    SCB_InvalidateDCache_by_Addr((uint32_t *)&obj->attr_rxdma.mem_buff[0], 
                                obj->attr_rxdma.mem_capacity);
#endif  // End With Define DRV_BSP_H7

    devbuf_write(&obj->rx_buf, &obj->attr_rxdma.mem_buff[0], obj->attr_rxdma.mem_capacity);
	HAL_UART_Receive_DMA(&obj->com, &obj->attr_rxdma.mem_buff[0], obj->attr_rxdma.mem_capacity);
}


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t idx = 0;
    if (huart->Instance == USART1)		idx = 0;
    else if (huart->Instance == USART2)	idx = 1;
    else if (huart->Instance == USART3)	idx = 2;
#if (BSP_CHIP_RESOURCE_LEVEL > 1)
    else if (huart->Instance == UART4)	idx = 3;
    else if (huart->Instance == UART5)	idx = 4;
    else if (huart->Instance == USART6)	idx = 5;
    else if (huart->Instance == UART7)	idx = 6;
    else if (huart->Instance == UART8)	idx = 7;
#endif  // End With Define BSP_CHIP_RESOURCE_LEVEL

    drv_uart_txdma_irq(drv_uart_list[idx]);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t idx = 0;
    if (huart->Instance == USART1)		idx = 0;
    else if (huart->Instance == USART2)	idx = 1;
    else if (huart->Instance == USART3)	idx = 2;
#if (BSP_CHIP_RESOURCE_LEVEL > 1)
    else if (huart->Instance == UART4)	idx = 3;
    else if (huart->Instance == UART5)	idx = 4;
    else if (huart->Instance == USART6)	idx = 5;
    else if (huart->Instance == UART7)	idx = 6;
    else if (huart->Instance == UART8)	idx = 7;
#endif  // End With Define BSP_CHIP_RESOURCE_LEVEL

    drv_uart_rxdma_irq(drv_uart_list[idx]);
}

void USART1_IRQHandler(void)
{
    drv_uart_irq(drv_uart_list[0]);
}

void USART2_IRQHandler(void)
{	
    drv_uart_irq(drv_uart_list[1]);
}

void USART3_IRQHandler(void)
{	
    drv_uart_irq(drv_uart_list[2]);
}

#if (BSP_CHIP_RESOURCE_LEVEL > 1)

void UART4_IRQHandler(void)
{	
    drv_uart_irq(drv_uart_list[3]);
}

void UART5_IRQHandler(void)
{
    drv_uart_irq(drv_uart_list[4]);
}

void USART6_IRQHandler(void)
{
    drv_uart_irq(drv_uart_list[5]);
}

void UART7_IRQHandler(void)
{
    drv_uart_irq(drv_uart_list[6]);
}

void UART8_IRQHandler(void)
{
    drv_uart_irq(drv_uart_list[7]);
}
#endif  // End With Define BSP_CHIP_RESOURCE_LEVEL

#if defined (DRV_BSP_H7)

void DMA1_Stream0_IRQHandler(void)
{  
    HAL_DMA_IRQHandler(drv_uart_list[0]->com.hdmatx);
}

void DMA1_Stream1_IRQHandler(void)
{  
    HAL_DMA_IRQHandler(drv_uart_list[1]->com.hdmatx);
}

void DMA1_Stream2_IRQHandler(void)
{  
    HAL_DMA_IRQHandler(drv_uart_list[2]->com.hdmatx);
}

void DMA1_Stream3_IRQHandler(void)
{  
    HAL_DMA_IRQHandler(drv_uart_list[3]->com.hdmatx);
}

void DMA1_Stream4_IRQHandler(void)
{  
    HAL_DMA_IRQHandler(drv_uart_list[4]->com.hdmatx);
}

void DMA1_Stream5_IRQHandler(void)
{  
    HAL_DMA_IRQHandler(drv_uart_list[5]->com.hdmatx);
}

void DMA1_Stream6_IRQHandler(void)
{  
    HAL_DMA_IRQHandler(drv_uart_list[6]->com.hdmatx);
}

void DMA1_Stream7_IRQHandler(void)
{  
    HAL_DMA_IRQHandler(drv_uart_list[7]->com.hdmatx);
}

void DMA2_Stream0_IRQHandler(void)
{
    HAL_DMA_IRQHandler(drv_uart_list[0]->com.hdmarx);
}

void DMA2_Stream1_IRQHandler(void)
{  
    HAL_DMA_IRQHandler(drv_uart_list[1]->com.hdmarx);
}

void DMA2_Stream2_IRQHandler(void)
{  
    HAL_DMA_IRQHandler(drv_uart_list[2]->com.hdmarx);
}

void DMA2_Stream3_IRQHandler(void)
{  
    HAL_DMA_IRQHandler(drv_uart_list[3]->com.hdmarx);	
}

void DMA2_Stream4_IRQHandler(void)
{  
    HAL_DMA_IRQHandler(drv_uart_list[4]->com.hdmarx);	
}

void DMA2_Stream5_IRQHandler(void)
{  
    HAL_DMA_IRQHandler(drv_uart_list[5]->com.hdmarx);
}

void DMA2_Stream6_IRQHandler(void)
{  
    HAL_DMA_IRQHandler(drv_uart_list[6]->com.hdmarx);
}

void DMA2_Stream7_IRQHandler(void)
{  
    HAL_DMA_IRQHandler(drv_uart_list[7]->com.hdmarx);
}

#endif  // End With Define DRV_BSP_H7


#if defined (DRV_BSP_F1)
void DMA1_Channel1_IRQHandler() 
{
}

void DMA1_Channel2_IRQHandler() 
{
    HAL_DMA_IRQHandler(drv_uart_list[2]->com.hdmatx);
}

void DMA1_Channel3_IRQHandler() 
{
    HAL_DMA_IRQHandler(drv_uart_list[2]->com.hdmarx);
}

void DMA1_Channel4_IRQHandler() 
{
    HAL_DMA_IRQHandler(drv_uart_list[0]->com.hdmatx);
}

void DMA1_Channel5_IRQHandler() 
{
    HAL_DMA_IRQHandler(drv_uart_list[0]->com.hdmarx);
}

void DMA1_Channel6_IRQHandler()
{
    HAL_DMA_IRQHandler(drv_uart_list[1]->com.hdmarx);
}

void DMA1_Channel7_IRQHandler() 
{
    HAL_DMA_IRQHandler(drv_uart_list[1]->com.hdmatx);
}

#endif  // End With Define DRV_BSP_F1
