#include <board_config.h>
#include <drv_uart.h>

void DMA1_Channel7_IRQHandler() 
{
    STM32_USART_DMA_TX_IRQ(1);
}

void DMA1_Channel6_IRQHandler() 
{
    STM32_USART_DMA_RX_IRQ(1);
}


