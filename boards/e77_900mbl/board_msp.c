#include <board_config.h>
#include <drv_uart.h>

void DMA1_Channel1_IRQHandler() 
{
    STM32_USART_DMA_TX_IRQ(1);
}

void DMA1_Channel2_IRQHandler() 
{
    STM32_USART_DMA_TX_IRQ(2);
}

void DMA2_Channel1_IRQHandler() 
{
    STM32_USART_DMA_RX_IRQ(1);
}

void DMA2_Channel2_IRQHandler() 
{
    STM32_USART_DMA_RX_IRQ(2);
}
