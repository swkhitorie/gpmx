#include <board_config.h>
#include <drv_uart.h>

void DMA1_Stream0_IRQHandler()
{
    STM32_USART_DMA_TX_IRQ(1);
}

void DMA2_Stream0_IRQHandler()
{
    STM32_USART_DMA_RX_IRQ(1);
}
