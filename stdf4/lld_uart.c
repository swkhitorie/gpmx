/**
 * low level driver for stm32f4 series, base on std periph library
 * module uart
*/

#include "lld_uart.h"
#include <stdlib.h>

lld_uart_t *mcu_uart_list[8] = {0, 0, 0, 0, 0, 0, 0, 0};

static uint8_t devbuf_txarray[8][512];
static uint8_t devbuf_rxarray[8][512];

void lld_uart_init(lld_uart_t *obj, uint8_t com, uint32_t baud, 
        bool remap, bool txdma_en, bool rxdma_en,
        uint16_t parity, uint16_t wordlen, uint16_t stopbits)
{
	devbuf_init(&obj->txbuf, &devbuf_txarray[com - 1][0], 512);
	devbuf_init(&obj->rxbuf, &devbuf_rxarray[com - 1][0], 512);

    obj->txdma_enable = ((com <= 6) ? txdma_en : false);
    obj->rxdma_enable = ((com <= 6) ? rxdma_en : false);
    
	USART_TypeDef *puart[8] = {USART1, USART2, USART3, UART4, UART5, USART6, UART7, UART8};
	obj->huart = puart[com - 1];
    obj->com = com;
    
    if (!remap) {
        /*                         UART1    UART2     UART3    UART4     UART5     UART6		UART7    UART8  */
		GPIO_TypeDef *txport[8] = {GPIOA,   GPIOA,    GPIOB,   GPIOC,    GPIOC,    GPIOC,		GPIOF,   GPIOE};
		uint16_t      txpin[8]  = {9,       2,        10,      10,       12,       6,   		7,       1};   
		
		GPIO_TypeDef *rxport[8] = {GPIOA,   GPIOA,    GPIOB,   GPIOC,    GPIOD,    GPIOC,		GPIOF,   GPIOE};
		uint16_t      rxpin[8]  = {10,      3,        11,      11,       2,        7,			6,       0};    
		
		uint8_t       gpioafx[8] = {GPIO_AF_USART1, GPIO_AF_USART2, GPIO_AF_USART3, GPIO_AF_UART4, GPIO_AF_UART5, GPIO_AF_USART6};
        lld_gpio_init(&obj->tx_pin, txport[com - 1], txpin[com - 1], GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_High_Speed, gpioafx[com - 1]);
        lld_gpio_init(&obj->rx_pin, rxport[com - 1], rxpin[com - 1], GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_High_Speed, gpioafx[com - 1]);
    } else {
		/*                         UART1      UART2    UART3    UART4    UART5   UART6   UART7	 UART8 */
		GPIO_TypeDef *txport[8] = {GPIOB,     GPIOD,   GPIOD,   GPIOD,   GPIOB,  GPIOG,  GPIOE,  GPIOJ};
		uint16_t      txpin[8] =  {6,         5,       8,       1,       13,     14,		 8,	 8    };   
		
		GPIO_TypeDef *rxport[8] = {GPIOB,     GPIOD,   GPIOD,   GPIOD,   GPIOB,  GPIOG,  GPIOE,  GPIOJ};
		uint16_t      rxpin[8] =  {7,         6,       9,       0,       12,     9,			 7,  9    };    
		
		uint8_t       gpioafx[8]  = {GPIO_AF_USART1, GPIO_AF_USART2, GPIO_AF_USART3, GPIO_AF_UART4, GPIO_AF_UART5, GPIO_AF_USART6, GPIO_AF_UART7, GPIO_AF_UART8};
        lld_gpio_init(&obj->tx_pin, txport[com - 1], txpin[com - 1], GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_High_Speed, gpioafx[com - 1]);
        lld_gpio_init(&obj->rx_pin, rxport[com - 1], rxpin[com - 1], GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_High_Speed, gpioafx[com - 1]);
    }

    IRQn_Type tmpIQRn[8] = {USART1_IRQn, USART2_IRQn, USART3_IRQn, UART4_IRQn, UART5_IRQn, USART6_IRQn, (IRQn_Type)0, (IRQn_Type)0}; 
	NVIC_InitTypeDef NVIC_InitStructure;  
	NVIC_InitStructure.NVIC_IRQChannel                   = tmpIQRn[com - 1];
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0; 
	NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);     
    
	if (obj->txdma_enable) {
		IRQn_Type tmpIRQn_DMA[6] = {DMA2_Stream7_IRQn, DMA1_Stream6_IRQn, 
            DMA1_Stream3_IRQn, DMA1_Stream4_IRQn,DMA1_Stream7_IRQn,DMA2_Stream6_IRQn};
		NVIC_InitStructure.NVIC_IRQChannel = tmpIRQn_DMA[com - 1];
		NVIC_Init(&NVIC_InitStructure); 
	}
	
	if(obj->rxdma_enable){
		IRQn_Type tmpIRQn_DMA[8] = {DMA2_Stream2_IRQn, DMA1_Stream5_IRQn, 
            DMA1_Stream1_IRQn, DMA1_Stream2_IRQn,DMA1_Stream0_IRQn,DMA2_Stream1_IRQn};
		NVIC_InitStructure.NVIC_IRQChannel = tmpIRQn_DMA[com - 1];
		NVIC_Init(&NVIC_InitStructure); 		
	}
    
	uint32_t rcc_uart[8] = {RCC_APB2Periph_USART1, RCC_APB1Periph_USART2, RCC_APB1Periph_USART3,
        RCC_APB1Periph_UART4, RCC_APB1Periph_UART5,RCC_APB2Periph_USART6,RCC_APB1Periph_UART7, RCC_APB1Periph_UART8}; 
	if (com == 1 || com == 6) {
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
	DMA_Stream_TypeDef *dma_stream[6] = {DMA2_Stream7,DMA1_Stream6,DMA1_Stream3, DMA1_Stream4,DMA1_Stream7,DMA2_Stream6};
    uint32_t dma_channel[8] = {DMA_Channel_4, DMA_Channel_4, DMA_Channel_4, DMA_Channel_4, DMA_Channel_4, DMA_Channel_5};
    uint32_t dma_tcflag[8] = {DMA_FLAG_TCIF7, DMA_FLAG_TCIF6, DMA_FLAG_TCIF3, DMA_FLAG_TCIF4,DMA_FLAG_TCIF7,DMA_FLAG_TCIF6};
    uint32_t dma_itgl[8] = {DMA_IT_TCIF7, DMA_IT_TCIF6, DMA_IT_TCIF3, DMA_IT_TCIF4, DMA_IT_TCIF7, DMA_IT_TCIF6};
    
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
    obj->txdma.buffer = (uint8_t *)malloc(sizeof(uint8_t) * USART_DMA_TX_BUFFER_SIZE);
	obj->txdma.stream = dma_stream[obj->com - 1];
	obj->txdma.channel = dma_channel[obj->com - 1];
	obj->txdma.flag_tc = dma_tcflag[obj->com - 1]; 
	obj->txdma.it_gl = dma_itgl[obj->com - 1];

	DMA_DeInit(obj->txdma.stream); 
	obj->txdma.init.DMA_Channel            = obj->txdma.channel;
	obj->txdma.init.DMA_PeripheralBaseAddr = (uint32_t)&obj->huart->DR;	      
	obj->txdma.init.DMA_Memory0BaseAddr    = (uint32_t)obj->txdma.buffer;         
	obj->txdma.init.DMA_DIR                = DMA_DIR_MemoryToPeripheral;			
	obj->txdma.init.DMA_BufferSize         = 0;                      
	obj->txdma.init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
	obj->txdma.init.DMA_MemoryInc          = DMA_MemoryInc_Enable;	
	obj->txdma.init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	obj->txdma.init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;	
	obj->txdma.init.DMA_Mode               = DMA_Mode_Normal ;       
	obj->txdma.init.DMA_Priority           = DMA_Priority_Low;
	obj->txdma.init.DMA_FIFOMode		   = DMA_Priority_Medium;
	obj->txdma.init.DMA_FIFOThreshold 		 = DMA_FIFOThreshold_Full;   
	obj->txdma.init.DMA_MemoryBurst    		 = DMA_MemoryBurst_Single;									
	obj->txdma.init.DMA_PeripheralBurst		 = DMA_PeripheralBurst_Single;												       
	DMA_Init(obj->txdma.stream, &obj->txdma.init);    

	DMA_ITConfig(obj->txdma.stream, DMA_IT_TC, ENABLE);   
	DMA_ITConfig(obj->txdma.stream, DMA_IT_TE, ENABLE);   
	USART_DMACmd(obj->huart, USART_DMAReq_Tx, ENABLE);          
	DMA_Cmd(obj->txdma.stream, DISABLE);	  
}

void lld_uart_rx_dma_init(lld_uart_t *obj)
{
	DMA_Stream_TypeDef *dma_stream[6] = {DMA2_Stream2,DMA1_Stream5,DMA1_Stream1,DMA1_Stream2,DMA1_Stream0,DMA2_Stream2};
    uint32_t dma_channel[8] = {DMA_Channel_4, DMA_Channel_4, DMA_Channel_4, DMA_Channel_4, DMA_Channel_4,DMA_Channel_5};
    uint32_t dma_tcflag[8] = {DMA_FLAG_TCIF2, DMA_FLAG_TCIF5, DMA_FLAG_TCIF1, DMA_FLAG_TCIF2,DMA_FLAG_TCIF0,DMA_FLAG_TCIF2};
    uint32_t dma_itgl[8] = {DMA_IT_TCIF2, DMA_IT_TCIF5, DMA_IT_TCIF1, DMA_IT_TCIF2, DMA_IT_TCIF0, DMA_IT_TCIF2};
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	if (obj->huart == USART1 || obj->huart == USART6) 
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
	obj->rxdma.buffer = (uint8_t *)malloc(sizeof(uint8_t) * USART_DMA_RX_BUFFER_SIZE);
	obj->rxdma.stream = dma_stream[obj->com - 1];
	obj->rxdma.channel = dma_channel[obj->com - 1];
	obj->rxdma.flag_tc = dma_tcflag[obj->com -1];
	obj->rxdma.it_gl = dma_itgl[obj->com - 1];

	DMA_DeInit(obj->rxdma.stream); 
	obj->rxdma.init.DMA_Channel			   = obj->rxdma.channel;
	obj->rxdma.init.DMA_PeripheralBaseAddr = (uint32_t)&obj->huart->DR;	      
	obj->rxdma.init.DMA_Memory0BaseAddr    = (uint32_t)obj->rxdma.buffer;         
	obj->rxdma.init.DMA_DIR                = DMA_DIR_PeripheralToMemory;	
	obj->rxdma.init.DMA_BufferSize         = USART_DMA_RX_BUFFER_SIZE;                      
	obj->rxdma.init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
	obj->rxdma.init.DMA_MemoryInc          = DMA_MemoryInc_Enable;	
	obj->rxdma.init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	obj->rxdma.init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;	
	obj->rxdma.init.DMA_Mode               = DMA_Mode_Normal ;       
	obj->rxdma.init.DMA_Priority           = DMA_Priority_Medium; 
	obj->rxdma.init.DMA_FIFOMode		    = DMA_FIFOMode_Disable;
	obj->rxdma.init.DMA_FIFOThreshold 		 = DMA_FIFOThreshold_3QuartersFull;
	obj->rxdma.init.DMA_MemoryBurst    		 = DMA_MemoryBurst_Single;							
	obj->rxdma.init.DMA_PeripheralBurst		 = DMA_PeripheralBurst_Single;     
	DMA_Init(obj->rxdma.stream, &obj->rxdma.init);    
	
	//DMA Interrupt Config
	DMA_ITConfig(obj->rxdma.stream, DMA_IT_TC, ENABLE);   
	DMA_ITConfig(obj->rxdma.stream, DMA_IT_TE, ENABLE);   
	USART_DMACmd(obj->huart, USART_DMAReq_Rx, ENABLE);          
	DMA_Cmd(obj->rxdma.stream, ENABLE);	  
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
            devbuf_write(&obj->rxbuf, &c, 1);
		}
	} else {
		// DMA Rx
		if (USART_GetITStatus(obj->huart, USART_IT_IDLE) != RESET) {
			uint16_t bytecount = obj->huart->SR;
			bytecount = obj->huart->DR;
			DMA_Cmd(obj->rxdma.stream, DISABLE);
			DMA_ClearITPendingBit(obj->rxdma.stream, obj->rxdma.it_gl);
			DMA_ClearFlag(obj->rxdma.stream, obj->rxdma.flag_tc);
			
			bytecount = USART_DMA_RX_BUFFER_SIZE - obj->rxdma.stream->NDTR;
			if (bytecount > 0 && bytecount < USART_DMA_RX_BUFFER_SIZE) {
                devbuf_write(&obj->rxbuf, &obj->rxdma.buffer[0], bytecount);
            }
			DMA_SetCurrDataCounter(obj->rxdma.stream, USART_DMA_RX_BUFFER_SIZE);
			DMA_Cmd(obj->rxdma.stream, ENABLE);
		}
	}
	
	
	//=============================  TX ==================================//
	if (!obj->txdma_enable) {
		if (USART_GetITStatus(obj->huart, USART_IT_TXE) != RESET || 
            USART_GetITStatus(obj->huart, USART_IT_TC) != RESET) {
            if (devbuf_size(&obj->txbuf) > 0) {
                devbuf_read(&obj->txbuf, &c, 1);
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
	
	if (DMA_GetFlagStatus(obj->txdma.stream, obj->txdma.flag_tc) == SET) {
		DMA_ClearITPendingBit(obj->txdma.stream, obj->txdma.it_gl);
		DMA_ClearFlag(obj->txdma.stream, obj->txdma.flag_tc);
		DMA_Cmd(obj->txdma.stream, DISABLE);
		
        if (devbuf_size(&obj->txbuf) > 0) {
            if (devbuf_size(&obj->txbuf) <= USART_DMA_TX_BUFFER_SIZE) {
                obj->txdma.stream->NDTR = (uint32_t)devbuf_size(&obj->txbuf);
                devbuf_read(&obj->txbuf, &obj->txdma.buffer[0], devbuf_size(&obj->txbuf));
            } else {
                obj->txdma.stream->NDTR = USART_DMA_TX_BUFFER_SIZE;
                devbuf_read(&obj->txbuf, &obj->txdma.buffer[0], USART_DMA_TX_BUFFER_SIZE);   
            }
            DMA_Cmd(obj->txdma.stream, ENABLE);
        } else {
            obj->tx_busy = false; 
        }
	}   
}

void lld_uart_irq_rxdma(lld_uart_t *obj)
{
	if(!obj->rxdma_enable) return;  
	
	if (DMA_GetFlagStatus(obj->rxdma.stream, obj->rxdma.flag_tc) == SET) {
		uint16_t bytecount = obj->huart->SR;
		bytecount = obj->huart->DR;
		DMA_ClearITPendingBit(obj->rxdma.stream, obj->rxdma.it_gl);
		DMA_ClearFlag(obj->rxdma.stream, obj->rxdma.flag_tc);
		DMA_Cmd(obj->rxdma.stream, DISABLE);
		
		bytecount = USART_DMA_RX_BUFFER_SIZE - obj->rxdma.stream->NDTR;
		if (bytecount > 0 && bytecount < USART_DMA_RX_BUFFER_SIZE) {
            devbuf_write(&obj->rxbuf, &obj->rxdma.buffer[0], bytecount);
        }
		DMA_SetCurrDataCounter(obj->rxdma.stream, bytecount);
		DMA_Cmd(obj->rxdma.stream, ENABLE);
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
            devbuf_write(&obj->txbuf, &p[0], size);
            if (obj->tx_busy) return false;
            obj->tx_busy = true;
            
            if (obj->txdma_enable) {
                USART_DMACmd(obj->huart, USART_DMAReq_Tx, ENABLE);
                
                if (devbuf_size(&obj->txbuf) <= USART_DMA_TX_BUFFER_SIZE) {
                    obj->txdma.stream->NDTR = (uint32_t)devbuf_size(&obj->txbuf);
                    devbuf_read(&obj->txbuf, &obj->txdma.buffer[0], devbuf_size(&obj->txbuf));
                } else {
                    obj->txdma.stream->NDTR = USART_DMA_TX_BUFFER_SIZE;
                    devbuf_read(&obj->txbuf, &obj->txdma.buffer[0], USART_DMA_TX_BUFFER_SIZE);   
                }
                DMA_Cmd(obj->txdma.stream, ENABLE);
            } else {
                USART_ClearITPendingBit(obj->huart, USART_IT_TC);
                USART_ClearITPendingBit(obj->huart,USART_IT_TXE);
                USART_ITConfig(obj->huart, USART_IT_TC, ENABLE);
                USART_GetFlagStatus(obj->huart, USART_FLAG_TC);
                devbuf_read(&obj->txbuf, &c, 1);
                USART_SendData(obj->huart, c);  
            }
            break;
        }
    default: return false;
    }
    return true;
}

