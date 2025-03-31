#include <drv_uart.h>

/****************************************************************************
 * STM32H7 DMA Interrupt Callback 
 ****************************************************************************/
#if defined (DRV_BSP_H7)
void DMA1_Stream0_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[0]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmatx);
}

void DMA1_Stream1_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[1]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmatx);
}

void DMA1_Stream2_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[2]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmatx);
}

void DMA1_Stream3_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[3]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmatx);
}

void DMA1_Stream4_IRQHandler(void)
{  
    struct up_uart_dev_s *priv = g_uart_list[4]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmatx);
}

void DMA1_Stream5_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[5]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmatx);
}

void DMA1_Stream6_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[6]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmatx);
}

void DMA1_Stream7_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[7]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmatx);
}

void DMA2_Stream0_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[0]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmarx);
}

void DMA2_Stream1_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[1]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmarx);
}

void DMA2_Stream2_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[2]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmarx);
}

void DMA2_Stream3_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[3]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmarx);	
}

void DMA2_Stream4_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[4]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmarx);	
}

void DMA2_Stream5_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[5]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmarx);
}

void DMA2_Stream6_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[6]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmarx);
}

void DMA2_Stream7_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[7]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmarx);
}
#endif

/****************************************************************************
 * STM32F1 DMA Interrupt Callback 
 ****************************************************************************/
#if defined (DRV_BSP_F1)
void DMA1_Channel1_IRQHandler() 
{
}

void DMA1_Channel2_IRQHandler() 
{
    struct up_uart_dev_s *priv = g_uart_list[2]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmatx);
}

void DMA1_Channel3_IRQHandler() 
{
    struct up_uart_dev_s *priv = g_uart_list[2]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmarx);
}

void DMA1_Channel4_IRQHandler() 
{
    struct up_uart_dev_s *priv = g_uart_list[0]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmatx);
}

void DMA1_Channel5_IRQHandler() 
{
    struct up_uart_dev_s *priv = g_uart_list[0]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmarx);
}

void DMA1_Channel6_IRQHandler()
{
    struct up_uart_dev_s *priv = g_uart_list[1]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmarx);
}

void DMA1_Channel7_IRQHandler() 
{
    struct up_uart_dev_s *priv = g_uart_list[1]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmatx);
}
#endif

/****************************************************************************
 * STM32F4 DMA Interrupt Callback 
 ****************************************************************************/
#if defined (DRV_BSP_F4)
void DMA1_Stream0_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[4]->priv;
    //DMA1_Stream0 -> UART5_RX+UART8_TX, prefer to UART5_RX
    HAL_DMA_IRQHandler(priv->com.hdmarx);
}

void DMA1_Stream1_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[2]->priv;
    //DMA1_Stream1 -> USART3_RX+UART7_TX, prefer to USART3_RX
    HAL_DMA_IRQHandler(priv->com.hdmarx);
}

void DMA1_Stream2_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[3]->priv;
    //UART4_RX
    HAL_DMA_IRQHandler(priv->com.hdmarx);
}

void DMA1_Stream3_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[2]->priv;
    //USART3_TX
    HAL_DMA_IRQHandler(priv->com.hdmatx);
}

void DMA1_Stream4_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[3]->priv;
    //USART4_TX
    HAL_DMA_IRQHandler(priv->com.hdmatx);
}

void DMA1_Stream5_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[1]->priv;
    //USART2_RX
    HAL_DMA_IRQHandler(priv->com.hdmarx);
}

void DMA1_Stream6_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[1]->priv;
    //USART2_TX
    HAL_DMA_IRQHandler(priv->com.hdmatx);
}

void DMA1_Stream7_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[4]->priv;
    //UART5_TX
    HAL_DMA_IRQHandler(priv->com.hdmatx);
}

void DMA2_Stream1_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[5]->priv;
    //USART6_RX
    HAL_DMA_IRQHandler(priv->com.hdmarx);
}

void DMA2_Stream2_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[0]->priv;
    //USART1_RX
    HAL_DMA_IRQHandler(priv->com.hdmarx);
}

void DMA2_Stream6_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[5]->priv;
    //USART6_TX
    HAL_DMA_IRQHandler(priv->com.hdmatx);
}

void DMA2_Stream7_IRQHandler(void)
{
    struct up_uart_dev_s *priv = g_uart_list[0]->priv;
    //USART1_TX
    HAL_DMA_IRQHandler(priv->com.hdmatx);
}
#endif

/****************************************************************************
 * STM32WL DMA Interrupt Callback 
 ****************************************************************************/
#if defined (DRV_BSP_WL)
void DMA1_Channel1_IRQHandler() 
{
    struct up_uart_dev_s *priv = g_uart_list[0]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmatx);
}

void DMA1_Channel2_IRQHandler() 
{
    struct up_uart_dev_s *priv = g_uart_list[1]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmatx);
}

void DMA2_Channel1_IRQHandler() 
{
    struct up_uart_dev_s *priv = g_uart_list[0]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmarx);
}

void DMA2_Channel2_IRQHandler() 
{
    struct up_uart_dev_s *priv = g_uart_list[1]->priv;
    HAL_DMA_IRQHandler(priv->com.hdmarx);
}
#endif

