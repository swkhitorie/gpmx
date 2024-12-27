/**
 * low level driver for stm32h7 series, base on cubehal library
 * module uart
*/

#include "include/lld_uart.h"

lld_uart_t *mcu_uart_list[8] = {0,0,0,0,0,0,0,0};

ALIGN_32BYTES(RAM_D1 static uint8_t dmaTxBuf[8][USART_DMA_TX_BUFFER_SIZE]);
ALIGN_32BYTES(RAM_D1 static uint8_t dmaRxBuf[8][USART_DMA_RX_BUFFER_SIZE]);
static uint8_t devbuf_txarray[8][512];
static uint8_t devbuf_rxarray[8][512];

bool lowlevel_uart_pin_source_init(lld_uart_t *obj, uint8_t uart_num, uint8_t uart_tx_selec, uint8_t uart_rx_selec)
{
	const struct pin_node *uart_tx_node;
	const struct pin_node *uart_rx_node;
	uint32_t is_pin_node_initable;
	
	switch (uart_num) {
	case 1:
		if (PINNODE(uint32_t)(USART_PINCTRL_SOURCE(1, UART_PIN_TX, uart_tx_selec)) != NULL && 
			PINNODE(uint32_t)(USART_PINCTRL_SOURCE(1, UART_PIN_RX, uart_rx_selec)) != NULL) {
			uart_tx_node = USART_PINCTRL_SOURCE(1, UART_PIN_TX, uart_tx_selec);
			uart_rx_node = USART_PINCTRL_SOURCE(1, UART_PIN_RX, uart_rx_selec);
			is_pin_node_initable = uart_tx_node->gpio_port && uart_rx_node->gpio_port;
		}else {
			return false;
		}
		break;
	case 2:
		if (PINNODE(uint32_t)(USART_PINCTRL_SOURCE(2, UART_PIN_TX, uart_tx_selec)) != NULL && 
			PINNODE(uint32_t)(USART_PINCTRL_SOURCE(2, UART_PIN_RX, uart_rx_selec)) != NULL) {
			uart_tx_node = USART_PINCTRL_SOURCE(2, UART_PIN_TX, uart_tx_selec);
			uart_rx_node = USART_PINCTRL_SOURCE(2, UART_PIN_RX, uart_rx_selec);
			is_pin_node_initable = uart_tx_node->gpio_port && uart_rx_node->gpio_port;
		}else {
			return false;
		}
		break;
	case 3:
		if (PINNODE(uint32_t)(USART_PINCTRL_SOURCE(3, UART_PIN_TX, uart_tx_selec)) != NULL && 
			PINNODE(uint32_t)(USART_PINCTRL_SOURCE(3, UART_PIN_RX, uart_rx_selec)) != NULL) {
			uart_tx_node = USART_PINCTRL_SOURCE(3, UART_PIN_TX, uart_tx_selec);
			uart_rx_node = USART_PINCTRL_SOURCE(3, UART_PIN_RX, uart_rx_selec);
			is_pin_node_initable = uart_tx_node->gpio_port && uart_rx_node->gpio_port;
		}else {
			return false;
		}
		break;
	case 4:
		if (PINNODE(uint32_t)(UART_PINCTRL_SOURCE(4, UART_PIN_TX, uart_tx_selec)) != NULL && 
			PINNODE(uint32_t)(UART_PINCTRL_SOURCE(4, UART_PIN_RX, uart_rx_selec)) != NULL) {
			uart_tx_node = UART_PINCTRL_SOURCE(4, UART_PIN_TX, uart_tx_selec);
			uart_rx_node = UART_PINCTRL_SOURCE(4, UART_PIN_RX, uart_rx_selec);
			is_pin_node_initable = uart_tx_node->gpio_port && uart_rx_node->gpio_port;
		}else {
			return false;
		}
		break;
	case 5:
		if (PINNODE(uint32_t)(UART_PINCTRL_SOURCE(5, UART_PIN_TX, uart_tx_selec)) != NULL && 
			PINNODE(uint32_t)(UART_PINCTRL_SOURCE(5, UART_PIN_RX, uart_rx_selec)) != NULL) {
			uart_tx_node = UART_PINCTRL_SOURCE(5, UART_PIN_TX, uart_tx_selec);
			uart_rx_node = UART_PINCTRL_SOURCE(5, UART_PIN_RX, uart_rx_selec);
			is_pin_node_initable = uart_tx_node->gpio_port && uart_rx_node->gpio_port;
		}else {
			return false;
		}
		break;
	case 6:
		if (PINNODE(uint32_t)(USART_PINCTRL_SOURCE(6, UART_PIN_TX, uart_tx_selec)) != NULL && 
			PINNODE(uint32_t)(USART_PINCTRL_SOURCE(6, UART_PIN_RX, uart_rx_selec)) != NULL) {
			uart_tx_node = USART_PINCTRL_SOURCE(6, UART_PIN_TX, uart_tx_selec);
			uart_rx_node = USART_PINCTRL_SOURCE(6, UART_PIN_RX, uart_rx_selec);
			is_pin_node_initable = uart_tx_node->gpio_port && uart_rx_node->gpio_port;
		}else {
			return false;
		}
		break;
	case 7:
		if (PINNODE(uint32_t)(UART_PINCTRL_SOURCE(7, UART_PIN_TX, uart_tx_selec)) != NULL && 
			PINNODE(uint32_t)(UART_PINCTRL_SOURCE(7, UART_PIN_RX, uart_rx_selec)) != NULL) {
			uart_tx_node = UART_PINCTRL_SOURCE(7, UART_PIN_TX, uart_tx_selec);
			uart_rx_node = UART_PINCTRL_SOURCE(7, UART_PIN_RX, uart_rx_selec);
			is_pin_node_initable = uart_tx_node->gpio_port && uart_rx_node->gpio_port;
		}else {
			return false;
		}
		break;
	case 8:
		if (PINNODE(uint32_t)(UART_PINCTRL_SOURCE(8, UART_PIN_TX, uart_tx_selec)) != NULL && 
			PINNODE(uint32_t)(UART_PINCTRL_SOURCE(8, UART_PIN_RX, uart_rx_selec)) != NULL) {
			uart_tx_node = UART_PINCTRL_SOURCE(8, UART_PIN_TX, uart_tx_selec);
			uart_rx_node = UART_PINCTRL_SOURCE(8, UART_PIN_RX, uart_rx_selec);
			is_pin_node_initable = uart_tx_node->gpio_port && uart_rx_node->gpio_port;
		}else {
			return false;
		}
		break;
	default: return false;
	}
	
	if (is_pin_node_initable != 0) {
        lld_gpio_init(&obj->tx_pin, uart_tx_node->gpio_port, uart_tx_node->pin_num, GPIO_MODE_AF_PP,
						GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH, uart_tx_node->alternate);
        lld_gpio_init(&obj->rx_pin, uart_rx_node->gpio_port, uart_rx_node->pin_num, GPIO_MODE_AF_PP,
						GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH, uart_rx_node->alternate);    
	}else {
		return false;
	}
	return true;
}

void lld_uart_tx_dma_init(lld_uart_t *obj)
{
    if (!obj->txdma_enable) return;
	obj->txdma.buffer = dmaTxBuf[obj->com - 1];
	__HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_LINKDMA(&obj->huart, hdmatx, obj->txdma.handle);
    
	DMA_Stream_TypeDef *txStream[8] = {DMA1_Stream0, DMA1_Stream1, DMA1_Stream2, 
		DMA1_Stream3, DMA1_Stream4, DMA1_Stream5, DMA1_Stream6, DMA1_Stream7};
	uint8_t txDMARequest[8] = {DMA_REQUEST_USART1_TX, DMA_REQUEST_USART2_TX, DMA_REQUEST_USART3_TX, 
		DMA_REQUEST_UART4_TX, DMA_REQUEST_UART5_TX, DMA_REQUEST_USART6_TX, DMA_REQUEST_UART7_TX, DMA_REQUEST_UART8_TX};
	
    HAL_DMA_DeInit(&obj->txdma.handle);
    obj->txdma.handle.Instance = txStream[obj->com - 1];
	obj->txdma.handle.Init.Request = txDMARequest[obj->com - 1];
	obj->txdma.handle.Init.Direction = DMA_MEMORY_TO_PERIPH;
	obj->txdma.handle.Init.PeriphInc = DMA_PINC_DISABLE;
	obj->txdma.handle.Init.MemInc = DMA_MINC_ENABLE;
	obj->txdma.handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	obj->txdma.handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	obj->txdma.handle.Init.Mode = DMA_NORMAL;
	obj->txdma.handle.Init.Priority = DMA_PRIORITY_MEDIUM;
	obj->txdma.handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	obj->txdma.handle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	obj->txdma.handle.Init.MemBurst=DMA_MBURST_SINGLE;
	obj->txdma.handle.Init.PeriphBurst=DMA_PBURST_SINGLE;
	HAL_DMA_Init(&obj->txdma.handle);
        
	IRQn_Type txDMAIRQn[8] = {DMA1_Stream0_IRQn, DMA1_Stream1_IRQn, DMA1_Stream2_IRQn, DMA1_Stream3_IRQn, DMA1_Stream4_IRQn, DMA1_Stream5_IRQn, DMA1_Stream6_IRQn, DMA1_Stream7_IRQn};
	HAL_NVIC_SetPriority(txDMAIRQn[obj->com - 1], 1, 0);
	HAL_NVIC_EnableIRQ(txDMAIRQn[obj->com - 1]);
}

void lld_uart_rx_dma_init(lld_uart_t *obj)
{
    if (!obj->rxdma_enable) return;
    obj->rxdma.buffer = dmaRxBuf[obj->com - 1];
	__HAL_RCC_DMA2_CLK_ENABLE();
    __HAL_LINKDMA(&obj->huart, hdmarx, obj->rxdma.handle);
    
	DMA_Stream_TypeDef *rxStream[8] = {DMA2_Stream0, DMA2_Stream1, DMA2_Stream2, 
		DMA2_Stream3, DMA2_Stream4, DMA2_Stream5, DMA2_Stream6, DMA2_Stream7};
	uint8_t rxDMARequest[8] = {DMA_REQUEST_USART1_RX, DMA_REQUEST_USART2_RX, DMA_REQUEST_USART3_RX, 
		DMA_REQUEST_UART4_RX, DMA_REQUEST_UART5_RX, DMA_REQUEST_USART6_RX, DMA_REQUEST_UART7_RX, DMA_REQUEST_UART8_RX};	
	
    HAL_DMA_DeInit(&obj->rxdma.handle);
    obj->rxdma.handle.Instance = rxStream[obj->com - 1];
	obj->rxdma.handle.Init.Request = rxDMARequest[obj->com - 1];
	obj->rxdma.handle.Init.Direction = DMA_PERIPH_TO_MEMORY;
	obj->rxdma.handle.Init.PeriphInc = DMA_PINC_DISABLE;
	obj->rxdma.handle.Init.MemInc = DMA_MINC_ENABLE;
	obj->rxdma.handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	obj->rxdma.handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	obj->rxdma.handle.Init.Mode = DMA_NORMAL;
	obj->rxdma.handle.Init.Priority = DMA_PRIORITY_MEDIUM;
	obj->rxdma.handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	obj->rxdma.handle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	obj->rxdma.handle.Init.MemBurst = DMA_MBURST_SINGLE;
	obj->rxdma.handle.Init.PeriphBurst = DMA_PBURST_SINGLE;
	HAL_DMA_Init(&obj->rxdma.handle);

    IRQn_Type rxDMAIRQn[8] = {DMA2_Stream0_IRQn, DMA2_Stream1_IRQn, DMA2_Stream2_IRQn, DMA2_Stream3_IRQn, DMA2_Stream4_IRQn, DMA2_Stream5_IRQn, DMA2_Stream6_IRQn, DMA2_Stream7_IRQn};
	HAL_NVIC_SetPriority(rxDMAIRQn[obj->com - 1], 1, 0);
	HAL_NVIC_EnableIRQ(rxDMAIRQn[obj->com - 1]);
    
    HAL_UART_Receive_DMA(&obj->huart, obj->rxdma.buffer, USART_DMA_RX_BUFFER_SIZE);
}

void lld_uart_init(lld_uart_t *obj, uint8_t com, uint32_t baud, 
        uint8_t tx_selec, uint8_t rx_selec, bool txdma_en,bool rxdma_en,
        uint16_t parity, uint16_t wordlen, uint16_t stopbits)
{
    obj->com = com;
    obj->baud = baud;
    obj->stopbits = stopbits;
    obj->parity = parity;
    obj->wordlen = wordlen;
    obj->txdma_enable = txdma_en;
    obj->rxdma_enable = rxdma_en;

	devbuf_init(&obj->txbuf, &devbuf_txarray[com - 1][0], 512);
	devbuf_init(&obj->rxbuf, &devbuf_rxarray[com - 1][0], 512);
    
	if (!lowlevel_uart_pin_source_init(obj, com, tx_selec, rx_selec)) return;

	/* Initialize UART */
	switch (com) {
	case 1:	__HAL_RCC_USART1_CLK_ENABLE();	break;
	case 2:	__HAL_RCC_USART2_CLK_ENABLE();	break;
	case 3:	__HAL_RCC_USART3_CLK_ENABLE();	break;
	case 4:	__HAL_RCC_UART4_CLK_ENABLE();	break;
	case 5:	__HAL_RCC_UART5_CLK_ENABLE();	break;
	case 6:	__HAL_RCC_USART6_CLK_ENABLE();	break;
	case 7:	__HAL_RCC_UART7_CLK_ENABLE();	break;
	case 8:	__HAL_RCC_UART8_CLK_ENABLE();	break;
	}
    
    USART_TypeDef *tmpUART[8] = {USART1, USART2, USART3, UART4, UART5, USART6, UART7, UART8};
	obj->huart.Instance = tmpUART[com - 1];
	obj->huart.Init.BaudRate = baud;
    obj->huart.Init.WordLength = wordlen;
    obj->huart.Init.StopBits = stopbits;
    obj->huart.Init.Parity = parity;
	obj->huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	obj->huart.Init.Mode = UART_MODE_TX_RX;
	HAL_UART_Init(&obj->huart);

    lld_uart_tx_dma_init(obj);
    lld_uart_rx_dma_init(obj);
    
	IRQn_Type uartIRQ[8] = {USART1_IRQn, USART2_IRQn, USART3_IRQn, UART4_IRQn, 
                            UART5_IRQn, USART6_IRQn, UART7_IRQn, UART8_IRQn};
	HAL_NVIC_SetPriority(uartIRQ[com - 1], 1, 2);
	HAL_NVIC_EnableIRQ(uartIRQ[com - 1]);
	__HAL_UART_ENABLE_IT(&obj->huart, UART_IT_IDLE);

	mcu_uart_list[com - 1] = obj;
	obj->tx_busy = false;
}

bool lld_uart_send_bytes(lld_uart_t *obj, const uint8_t *p, uint16_t size, lld_rwway way)
{
    switch (way) {
    case RWPOLL:
        HAL_UART_Transmit(&obj->huart, (uint8_t *)p, size, 3000);
        break;
    case RWIT:
    case RWDMA:
        {
            devbuf_write(&obj->txbuf, &p[0], size);
            if (obj->tx_busy) return false;
            obj->tx_busy = true;
            if (obj->txdma_enable) {
                uint16_t bufsize = (uint32_t)devbuf_size(&obj->txbuf);
                if (bufsize <= USART_DMA_TX_BUFFER_SIZE) {
                    devbuf_read(&obj->txbuf, &obj->txdma.buffer[0], bufsize);
                    SCB_CleanDCache_by_Addr((uint32_t *)&obj->txdma.buffer[0], USART_DMA_TX_BUFFER_SIZE);
                    HAL_UART_Transmit_DMA(&obj->huart, &obj->txdma.buffer[0], bufsize);
                } else {
                    devbuf_read(&obj->txbuf, &obj->txdma.buffer[0], USART_DMA_TX_BUFFER_SIZE);
                    SCB_CleanDCache_by_Addr((uint32_t *)&obj->txdma.buffer[0], USART_DMA_TX_BUFFER_SIZE);
                    HAL_UART_Transmit_DMA(&obj->huart, &obj->txdma.buffer[0], USART_DMA_TX_BUFFER_SIZE);  
                }
            } else {
                uint16_t bufsize = (uint32_t)devbuf_size(&obj->txbuf);
                HAL_UART_Transmit_IT(&obj->huart, &obj->txbuf.buf[0], bufsize);
            }
            break; 
        }
    default: false;
    }
    return true;
}

void lld_uart_irq(lld_uart_t *obj)
{
	__HAL_UART_DISABLE_IT(&obj->huart, UART_IT_ERR);
	__HAL_UART_DISABLE_IT(&obj->huart, UART_IT_PE);

	if ((__HAL_UART_GET_FLAG(&obj->huart, UART_FLAG_IDLE) != RESET)) {
		__HAL_UART_CLEAR_IDLEFLAG(&obj->huart);
		uint32_t tmp = obj->huart.Instance->ISR;
		tmp = obj->huart.Instance->RDR;
		HAL_UART_AbortReceive(&obj->huart);
        
		uint16_t rxlen =  USART_DMA_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(obj->huart.hdmarx);
		
		/* save SRAM(DMA) data to D-Cache data */
		SCB_InvalidateDCache_by_Addr((uint32_t *)&obj->rxdma.buffer[0], USART_DMA_RX_BUFFER_SIZE);
        devbuf_write(&obj->rxbuf, &obj->rxdma.buffer[0], rxlen);
		HAL_UART_Receive_DMA(&obj->huart, obj->rxdma.buffer, USART_DMA_RX_BUFFER_SIZE);
	}
	
	/* Call HAL function for other UART IRQ (such as TC IRQ and RXNE IRQ) */
	HAL_UART_IRQHandler(&obj->huart); 
}

void lld_uart_irq_txdma(lld_uart_t *obj)
{
	uint32_t len = devbuf_size(&obj->txbuf);

	// when usart6 dma tx function break here, bug will be corrected
	if (len > 0) {
		if(len > USART_DMA_TX_BUFFER_SIZE) len = USART_DMA_TX_BUFFER_SIZE;
        
        devbuf_read(&obj->txbuf, &obj->txdma.buffer[0], len);
		obj->tx_busy = true;
		/* save SRAM(DMA) data to D-Cache data */
		SCB_InvalidateDCache_by_Addr((uint32_t *)&obj->txdma.buffer[0], USART_DMA_TX_BUFFER_SIZE);
		HAL_UART_Transmit_DMA(&obj->huart, &obj->txdma.buffer[0], len);
	}else {
		obj->tx_busy = false; 
	}
}

void lld_uart_irq_rxdma(lld_uart_t *obj)
{
	HAL_UART_AbortReceive(&obj->huart);
	
	/* save SRAM(DMA) data to D-Cache data */
	SCB_InvalidateDCache_by_Addr((uint32_t *)&obj->rxdma.buffer[0], USART_DMA_RX_BUFFER_SIZE);
    devbuf_write(&obj->rxbuf, &obj->rxdma.buffer[0], USART_DMA_RX_BUFFER_SIZE);
	HAL_UART_Receive_DMA(&obj->huart, &obj->rxdma.buffer[0], USART_DMA_RX_BUFFER_SIZE);
}



/*****************************************************************
 *****  CubeH7 UART interrupt and callback
 ****************************************************************
*/

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t idx = 0;
    
    if (huart->Instance == USART1)		idx = 0;
    else if (huart->Instance == USART2)	idx = 1;
    else if (huart->Instance == USART3)	idx = 2;
    else if (huart->Instance == UART4)	idx = 3;
    else if (huart->Instance == UART5)	idx = 4;
    else if (huart->Instance == USART6)	idx = 5;
    else if (huart->Instance == UART7)	idx = 6;
    else if (huart->Instance == UART8)	idx = 7;

    if (mcu_uart_list[idx]) {
        lld_uart_irq_txdma(mcu_uart_list[idx]);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    static uint8_t idx = 0;
    
    if (huart->Instance == USART1)		idx = 0;
    else if (huart->Instance == USART2)	idx = 1;
    else if (huart->Instance == USART3)	idx = 2;
    else if (huart->Instance == UART4)	idx = 3;
    else if (huart->Instance == UART5)	idx = 4;
    else if (huart->Instance == USART6)	idx = 5;
    else if (huart->Instance == UART7)	idx = 6;
    else if (huart->Instance == UART8)	idx = 7;

    if (mcu_uart_list[idx]) {
        lld_uart_irq_rxdma(mcu_uart_list[idx]);
    }
}
/**
 * @brief UARTx_IRQHandler
*/
/** UART1 IRQ */
void USART1_IRQHandler(void)
{
    if(mcu_uart_list[0])
        lld_uart_irq(mcu_uart_list[0]);
}

/** UART2 IRQ */
void USART2_IRQHandler(void)
{	
    if(mcu_uart_list[1])	
        lld_uart_irq(mcu_uart_list[1]);
}

/** UART3 IRQ */
void USART3_IRQHandler(void)
{	
    if(mcu_uart_list[2])	
        lld_uart_irq(mcu_uart_list[2]);
}

/** UART4 IRQ */
void UART4_IRQHandler(void)
{	
    if(mcu_uart_list[3])	
        lld_uart_irq(mcu_uart_list[3]);
}

/** UART5 IRQ */
void UART5_IRQHandler(void)
{
    if(mcu_uart_list[4])	
        lld_uart_irq(mcu_uart_list[4]);
}

/** UART6 IRQ */
void USART6_IRQHandler(void)
{
    if(mcu_uart_list[5])	
        lld_uart_irq(mcu_uart_list[5]);
}

/** UART7 IRQ */
void UART7_IRQHandler(void)
{
    if(mcu_uart_list[6])	
        lld_uart_irq(mcu_uart_list[6]);
}

/** UART8 IRQ */
void UART8_IRQHandler(void)
{
    if(mcu_uart_list[7])	
        lld_uart_irq(mcu_uart_list[7]);
}


/*****************************************************************
 *****  CubeH7 DMA interrupt and callback
 ****************************************************************
*/

/** DMA1 Stream0  IRQ */
void DMA1_Stream0_IRQHandler(void)
{  
    if(mcu_uart_list[0])	
        HAL_DMA_IRQHandler(mcu_uart_list[0]->huart.hdmatx);
}

/** DMA1 Stream1  IRQ */
void DMA1_Stream1_IRQHandler(void)
{  
    if(mcu_uart_list[1])	
        HAL_DMA_IRQHandler(mcu_uart_list[1]->huart.hdmatx);
}

/** DMA1 Stream2  IRQ */
void DMA1_Stream2_IRQHandler(void)
{  
    if(mcu_uart_list[2])	
        HAL_DMA_IRQHandler(mcu_uart_list[2]->huart.hdmatx);
}

/** DMA1 Stream3  IRQ */
void DMA1_Stream3_IRQHandler(void)
{  
    if(mcu_uart_list[3])	
        HAL_DMA_IRQHandler(mcu_uart_list[3]->huart.hdmatx);
}

/** DMA1 Stream4  IRQ */
void DMA1_Stream4_IRQHandler(void)
{  
    if(mcu_uart_list[4])	
        HAL_DMA_IRQHandler(mcu_uart_list[4]->huart.hdmatx);
}

/** DMA1 Stream5  IRQ */
void DMA1_Stream5_IRQHandler(void)
{  
    if(mcu_uart_list[5])	
        HAL_DMA_IRQHandler(mcu_uart_list[5]->huart.hdmatx);
}

/** DMA1 Stream6  IRQ */
void DMA1_Stream6_IRQHandler(void)
{  
    if(mcu_uart_list[6])	
        HAL_DMA_IRQHandler(mcu_uart_list[6]->huart.hdmatx);
}

/** DMA1 Stream7  IRQ */
void DMA1_Stream7_IRQHandler(void)
{  
    if(mcu_uart_list[7])	
        HAL_DMA_IRQHandler(mcu_uart_list[7]->huart.hdmatx);
}


/** DMA2 Stream0  IRQ */
void DMA2_Stream0_IRQHandler(void)
{  
    if(mcu_uart_list[0])	
        HAL_DMA_IRQHandler(mcu_uart_list[0]->huart.hdmarx);
}

/** DMA2 Stream1  IRQ */
void DMA2_Stream1_IRQHandler(void)
{  
    if(mcu_uart_list[1])	
        HAL_DMA_IRQHandler(mcu_uart_list[1]->huart.hdmarx);
}

/** DMA2 Stream2  IRQ */
void DMA2_Stream2_IRQHandler(void)
{  
    if(mcu_uart_list[2])	
        HAL_DMA_IRQHandler(mcu_uart_list[2]->huart.hdmarx);
}

/** DMA2 Stream3  IRQ */
void DMA2_Stream3_IRQHandler(void)
{  
    if(mcu_uart_list[3])	
        HAL_DMA_IRQHandler(mcu_uart_list[3]->huart.hdmarx);	
}

/** DMA2 Stream4  IRQ */
void DMA2_Stream4_IRQHandler(void)
{  
    if(mcu_uart_list[4])	
        HAL_DMA_IRQHandler(mcu_uart_list[4]->huart.hdmarx);	
}

/** DMA2 Stream5  IRQ */
void DMA2_Stream5_IRQHandler(void)
{  
    if(mcu_uart_list[5])	
        HAL_DMA_IRQHandler(mcu_uart_list[5]->huart.hdmarx);	
}

/** DMA2 Stream6  IRQ */
void DMA2_Stream6_IRQHandler(void)
{  
    if(mcu_uart_list[6])	
        HAL_DMA_IRQHandler(mcu_uart_list[6]->huart.hdmarx);	
}

/** DMA2 Stream7  IRQ */
void DMA2_Stream7_IRQHandler(void)
{  
    if(mcu_uart_list[7])	
        HAL_DMA_IRQHandler(mcu_uart_list[7]->huart.hdmarx);	
}
