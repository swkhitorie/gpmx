/**
 * low level driver for stm32f1 series, base on std periph library
 * module uart
*/

#include "lld_uart.h"
#include <stdlib.h>

lld_uart_t *mcu_uart_list[5] = {0, 0, 0, 0, 0};

void lld_uart_init(lld_uart_t *obj, uint8_t com, uint32_t baud, 
        bool remap, bool txdma_en, bool rxdma_en,
        uint16_t parity, uint16_t wordlen, uint16_t stopbits)
{
    if (!devfifo_init(&obj->txbuf, NULL, 512, DEVFIFO_ALLOC)) return;
    if (!devfifo_init(&obj->rxbuf, NULL, 512, DEVFIFO_ALLOC)) return;
    
    obj->txdma_enable = ((com <= 6) ? txdma_en : false);
    obj->rxdma_enable = ((com <= 6) ? rxdma_en : false);
    
	USART_TypeDef *puart[5] = {USART1, USART2, USART3, UART4, UART5};
	obj->huart = puart[com - 1];
    obj->com = com;
    
    if (!remap) {
        /*                         UART1    UART2     UART3    UART4     UART5 */
		GPIO_TypeDef *txport[5] = {GPIOA,   GPIOA,    GPIOB,   GPIOC,    GPIOC};
		uint16_t      txpin[5]  = {9,       2,        10,      10,       12   };   
		
		GPIO_TypeDef *rxport[5] = {GPIOA,   GPIOA,    GPIOB,   GPIOC,    GPIOD};
		uint16_t      rxpin[5]  = {10,      3,        11,      11,       2};    
        lld_gpio_init(&obj->tx_pin, txport[com - 1], txpin[com - 1], GPIO_Mode_AF_PP, GPIO_Speed_50MHz, true);
        lld_gpio_init(&obj->rx_pin, rxport[com - 1], rxpin[com - 1], GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, true);
        
    } else if (com <= 3 && remap) {
		/*                         UART1      UART2    UART3 */
		GPIO_TypeDef *txport[8] = {GPIOB,     GPIOD,   GPIOD};
		uint16_t      txpin[8] =  {6,         5,       8    };   
		
		GPIO_TypeDef *rxport[8] = {GPIOB,     GPIOD,   GPIOD};
		uint16_t      rxpin[8] =  {7,         6,       9    };
        
        uint32_t      remapcfg[3] = {GPIO_Remap_USART1, GPIO_Remap_USART2, GPIO_FullRemap_USART3};
        lld_gpio_init(&obj->tx_pin, txport[com - 1], txpin[com - 1], GPIO_Mode_AF_PP, GPIO_Speed_50MHz, true);
        lld_gpio_init(&obj->rx_pin, rxport[com - 1], rxpin[com - 1], GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, true);
        GPIO_PinRemapConfig(remapcfg[com - 1],ENABLE);
    }

    IRQn_Type tmpIQRn[8] = {USART1_IRQn, USART2_IRQn, USART3_IRQn, UART4_IRQn, UART5_IRQn}; 
	NVIC_InitTypeDef NVIC_InitStructure;  
	NVIC_InitStructure.NVIC_IRQChannel                   = tmpIQRn[com - 1];
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0; 
	NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);     
    
	if (obj->txdma_enable) {
		IRQn_Type tmpIRQn_DMA[4] = {DMA1_Channel4_IRQn, DMA1_Channel7_IRQn, DMA1_Channel2_IRQn,  (IRQn_Type)59};
		NVIC_InitStructure.NVIC_IRQChannel = tmpIRQn_DMA[com - 1];
		NVIC_Init(&NVIC_InitStructure); 
	}
    
//	if(obj->rxdma_enable){
//		IRQn_Type tmpIRQn_DMA[8] = {DMA1_Channel5_IRQn, DMA1_Channel6_IRQn, DMA1_Channel3_IRQn,  IRQn(58)};
//		NVIC_InitStructure.NVIC_IRQChannel = tmpIRQn_DMA[com - 1];
//		NVIC_Init(&NVIC_InitStructure); 		
//	}
    
	uint32_t rcc_uart[5] = {RCC_APB2Periph_USART1, RCC_APB1Periph_USART2, RCC_APB1Periph_USART3, RCC_APB1Periph_UART4, RCC_APB1Periph_UART5}; 
	if (com == 1) {
        RCC_APB2PeriphClockCmd(rcc_uart[com - 1], ENABLE);
    } else {
        RCC_APB1PeriphClockCmd(rcc_uart[com - 1], ENABLE);
    }
	USART_InitTypeDef USART_InitStructure;    
	USART_InitStructure.USART_BaudRate            = baud;     
	USART_InitStructure.USART_WordLength          = wordlen;  
	USART_InitStructure.USART_StopBits            = stopbits; 
	USART_InitStructure.USART_Parity              = parity; 
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx; 
	USART_Init(obj->huart, &USART_InitStructure);  
	USART_Cmd(obj->huart, ENABLE); 
    
	if (obj->rxdma_enable) {
		USART_ITConfig(obj->huart, USART_IT_RXNE, DISABLE);
		USART_ITConfig(obj->huart, USART_IT_IDLE, ENABLE);
	} else {
        USART_ITConfig(obj->huart, USART_IT_RXNE, ENABLE);
    }
    
	if (obj->txdma_enable) { 
        USART_ITConfig(obj->huart, USART_IT_TC, DISABLE);
	}

	USART_ClearFlag(obj->huart, USART_FLAG_TC);
	USART_ClearFlag(obj->huart, USART_IT_RXNE);
	USART_ClearFlag(obj->huart, USART_IT_IDLE);

	if (obj->rxdma_enable) {
        lld_uart_rx_dma_init(obj);
	}
	
	if (obj->txdma_enable) {
        lld_uart_tx_dma_init(obj);
	}
    
	mcu_uart_list[com - 1] = obj;
}

void lld_uart_tx_dma_init(lld_uart_t *obj)
{
    DMA_Channel_TypeDef *dma_channel[4] = {DMA1_Channel4, DMA1_Channel7, DMA1_Channel2,  DMA2_Channel5};
    uint32_t dma_tcflag[4] = {DMA1_FLAG_TC4, DMA1_FLAG_TC7, DMA1_FLAG_TC2, DMA2_FLAG_TC5};
    uint32_t dma_itgl[4] = {DMA1_IT_GL4, DMA1_IT_GL7, DMA1_IT_GL2, DMA2_IT_GL5};
    
    if (obj->com < 4) {
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    } else {
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
    }
    
    obj->txdma.buffer = (uint8_t *)malloc(sizeof(uint8_t) * USART_DMA_TX_BUFFER_SIZE);
	obj->txdma.channel = dma_channel[obj->com - 1];
	obj->txdma.flag_tc = dma_tcflag[obj->com - 1]; 
	obj->txdma.it_gl = dma_itgl[obj->com - 1];

	DMA_DeInit(obj->txdma.channel); 
	obj->txdma.init.DMA_PeripheralBaseAddr = (uint32_t)&obj->huart->DR;	      
	obj->txdma.init.DMA_MemoryBaseAddr     = (uint32_t)obj->txdma.buffer;         
	obj->txdma.init.DMA_DIR                = DMA_DIR_PeripheralDST;			
	obj->txdma.init.DMA_BufferSize         = 0;                      
	obj->txdma.init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
	obj->txdma.init.DMA_MemoryInc          = DMA_MemoryInc_Enable;	
	obj->txdma.init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	obj->txdma.init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;	
	obj->txdma.init.DMA_Mode               = DMA_Mode_Normal;       
	obj->txdma.init.DMA_Priority           = DMA_Priority_Low;
    obj->txdma.init.DMA_M2M                = DMA_M2M_Disable;
	DMA_Init(obj->txdma.channel, &obj->txdma.init);    

	DMA_ITConfig(obj->txdma.channel, DMA_IT_TC, ENABLE);
	DMA_ITConfig(obj->txdma.channel, DMA_IT_TE, ENABLE);
	USART_DMACmd(obj->huart, USART_DMAReq_Tx, ENABLE);
	DMA_Cmd(obj->txdma.channel, DISABLE);
}

void lld_uart_rx_dma_init(lld_uart_t *obj)
{
    DMA_Channel_TypeDef *dma_channel[8] = {DMA1_Channel5, DMA1_Channel6, DMA1_Channel3,  DMA2_Channel3};
    uint32_t dma_tcflag[8] = {DMA1_FLAG_TC5, DMA1_FLAG_TC6, DMA1_FLAG_TC3, DMA2_FLAG_TC3};
    uint32_t dma_itgl[8] = {DMA1_IT_GL5, DMA1_IT_GL6, DMA1_IT_GL3, DMA2_IT_GL3};
    
    if (obj->com < 4) {
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    } else {
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE); 
    }
    
	obj->rxdma.buffer = (uint8_t *)malloc(sizeof(uint8_t) * USART_DMA_RX_BUFFER_SIZE);
	obj->rxdma.channel = dma_channel[obj->com - 1];
	obj->rxdma.flag_tc = dma_tcflag[obj->com -1];
	obj->rxdma.it_gl = dma_itgl[obj->com - 1];

	DMA_DeInit(obj->rxdma.channel); 
	obj->rxdma.init.DMA_PeripheralBaseAddr = (uint32_t)&obj->huart->DR;	      
	obj->rxdma.init.DMA_MemoryBaseAddr     = (uint32_t)obj->rxdma.buffer;         
	obj->rxdma.init.DMA_DIR                = DMA_DIR_PeripheralSRC;	
	obj->rxdma.init.DMA_BufferSize         = USART_DMA_RX_BUFFER_SIZE;                      
	obj->rxdma.init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
	obj->rxdma.init.DMA_MemoryInc          = DMA_MemoryInc_Enable;	
	obj->rxdma.init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	obj->rxdma.init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;	
	obj->rxdma.init.DMA_Mode               = DMA_Mode_Normal;    
	obj->rxdma.init.DMA_Priority           = DMA_Priority_Medium; 
    obj->rxdma.init.DMA_M2M                = DMA_M2M_Disable; 
	DMA_Init(obj->rxdma.channel, &obj->rxdma.init);    
	
	//DMA Interrupt Config
	//DMA_ITConfig(obj->rxdma.channel, DMA_IT_TC, ENABLE);   
	//DMA_ITConfig(obj->rxdma.channel, DMA_IT_TE, ENABLE);   
	USART_DMACmd(obj->huart, USART_DMAReq_Rx, ENABLE);          
	DMA_Cmd(obj->rxdma.channel, ENABLE);	  
}

void lld_uart_irq(lld_uart_t *obj)
{
    uint8_t c;
	if (USART_GetFlagStatus(obj->huart, USART_FLAG_ORE) != RESET) {
		USART_ReceiveData(obj->huart);
	}
	
	//=============================  RX ==================================//
	if (!obj->rxdma_enable) {
		// Interrupt Rx
		if (USART_GetITStatus(obj->huart, USART_IT_RXNE) != RESET) {
			USART_ClearITPendingBit(obj->huart, USART_IT_RXNE);
            c = USART_ReceiveData(obj->huart);
            devfifo_write(&obj->rxbuf, &c, 1);
		}
	} else {
		// DMA Rx
		if (USART_GetITStatus(obj->huart, USART_IT_IDLE) != RESET) {
			uint16_t bytecount = obj->huart->SR;
			bytecount = obj->huart->DR;
			DMA_ClearITPendingBit(obj->rxdma.it_gl);
			DMA_ClearFlag(obj->rxdma.flag_tc);
			DMA_Cmd(obj->rxdma.channel, DISABLE);
            
			bytecount = USART_DMA_RX_BUFFER_SIZE - obj->rxdma.channel->CNDTR;
			if (bytecount > 0 && bytecount < USART_DMA_RX_BUFFER_SIZE) {
                devfifo_write(&obj->rxbuf, &obj->rxdma.buffer[0], bytecount);
            }
			DMA_SetCurrDataCounter(obj->rxdma.channel, USART_DMA_RX_BUFFER_SIZE);
			DMA_Cmd(obj->rxdma.channel, ENABLE);
		}
	}
	
	
	//=============================  TX ==================================//
	if (!obj->txdma_enable) {
		if (USART_GetITStatus(obj->huart, USART_IT_TXE) != RESET || 
            USART_GetITStatus(obj->huart, USART_IT_TC) != RESET) {
            if (devfifo_size(&obj->txbuf) > 0) {
                devfifo_read(&obj->txbuf, &c, 1);
                obj->huart->DR = (c & (uint16_t)0x01FF);
            } else {
				USART_ITConfig(obj->huart, USART_IT_TXE, DISABLE);
				USART_ITConfig(obj->huart, USART_IT_TC, DISABLE);
				obj->tx_busy = false; 
            }
		}
	}
}

void lld_uart_irq_txdma(lld_uart_t *obj)
{
	if (!obj->txdma_enable) return;
	
	if (DMA_GetFlagStatus(obj->txdma.flag_tc) == SET) {
		DMA_ClearITPendingBit(obj->txdma.it_gl);
		DMA_ClearFlag(obj->txdma.flag_tc);
		DMA_Cmd(obj->txdma.channel, DISABLE);
		
        if (devfifo_size(&obj->txbuf) > 0) {
            if (devfifo_size(&obj->txbuf) <= USART_DMA_TX_BUFFER_SIZE) {
                obj->txdma.channel->CNDTR = (uint32_t)devfifo_size(&obj->txbuf);
                devfifo_read(&obj->txbuf, &obj->txdma.buffer[0], devfifo_size(&obj->txbuf));
            } else {
                obj->txdma.channel->CNDTR = USART_DMA_TX_BUFFER_SIZE;
                devfifo_read(&obj->txbuf, &obj->txdma.buffer[0], USART_DMA_TX_BUFFER_SIZE);   
            }
            DMA_Cmd(obj->txdma.channel, ENABLE);
        } else {
            obj->tx_busy = false; 
        }
	}   
}

void lld_uart_irq_rxdma(lld_uart_t *obj)
{
	if(!obj->rxdma_enable) return;  
	
	if (DMA_GetFlagStatus(obj->rxdma.flag_tc) == SET) {
		uint16_t bytecount = obj->huart->SR;
		bytecount = obj->huart->DR;
		DMA_ClearITPendingBit(obj->rxdma.it_gl);
		DMA_ClearFlag(obj->rxdma.flag_tc);
		DMA_Cmd(obj->rxdma.channel, DISABLE);
		
		bytecount = USART_DMA_RX_BUFFER_SIZE - obj->rxdma.channel->CNDTR;
		if (bytecount > 0 && bytecount < USART_DMA_RX_BUFFER_SIZE) {
            devfifo_write(&obj->rxbuf, &obj->rxdma.buffer[0], bytecount);
        }
		DMA_SetCurrDataCounter(obj->rxdma.channel, bytecount);
		DMA_Cmd(obj->rxdma.channel, ENABLE);
	}  
}

bool lld_uart_send_bytes(lld_uart_t *obj, const uint8_t *p, uint16_t size, lld_rwway way)
{
    uint8_t c = 0;
    switch (way) {
    case RWPOLL:
        {
            for(int i = 0; i < size; i++) {
                while(USART_GetFlagStatus(obj->huart, USART_FLAG_TXE) == RESET);
                USART_SendData(obj->huart, p[i]);
            }
            break;  
        }
    case RWIT:
    case RWDMA:
        {
            devfifo_write(&obj->txbuf, &p[0], size);
            if (obj->tx_busy) return false;
            obj->tx_busy = true;
            
            if (obj->txdma_enable) { 
                USART_DMACmd(obj->huart, USART_DMAReq_Tx, ENABLE);
                
                if (devfifo_size(&obj->txbuf) <= USART_DMA_TX_BUFFER_SIZE) {
                    obj->txdma.channel->CNDTR = (uint32_t)devfifo_size(&obj->txbuf);
                    devfifo_read(&obj->txbuf, &obj->txdma.buffer[0], devfifo_size(&obj->txbuf));
                } else {
                    obj->txdma.channel->CNDTR = USART_DMA_TX_BUFFER_SIZE;
                    devfifo_read(&obj->txbuf, &obj->txdma.buffer[0], USART_DMA_TX_BUFFER_SIZE);   
                }
                DMA_Cmd(obj->txdma.channel, ENABLE);
            } else {
                USART_ClearITPendingBit(obj->huart, USART_IT_TC);
                USART_ClearITPendingBit(obj->huart,USART_IT_TXE);
                USART_ITConfig(obj->huart, USART_IT_TC, ENABLE);
                USART_GetFlagStatus(obj->huart, USART_FLAG_TC);
                devfifo_read(&obj->txbuf, &c, 1);
                USART_SendData(obj->huart, c);  
            }
            break;
        }
    default: return false;
    }
    return true;
}

