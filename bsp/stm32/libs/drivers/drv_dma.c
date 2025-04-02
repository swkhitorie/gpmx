#include <drv_uart.h>

/****************************************************************************
 * STM32H7 DMA Interrupt Callback 
 ****************************************************************************/
/*
H7 series dma can be freely configured
all dma configured into usart default

|    DMA1    | Stream 0         | Stream 1         | Stream 2         | Stream 3         | Stream 4         | Stream 5         | Stream 6         | Stream 7         |
|------------|------------------|------------------|------------------|------------------|------------------|------------------|------------------|------------------|
|            | USART1_TX        | USART2_TX        | USART3_TX        | UART4_TX         | UART5_TX         | USART6_TX        | UART7_TX         | UART8_TX         |

|    DMA2    | Stream 0         | Stream 1         | Stream 2         | Stream 3         | Stream 4         | Stream 5         | Stream 6         | Stream 7         |
|------------|------------------|------------------|------------------|------------------|------------------|------------------|------------------|------------------|
|            | USART1_RX        | USART2_RX        | USART3_RX        | UART4_RX         | UART5_RX         | USART6_RX        | UART7_RX         | UART8_RX         |
 */

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
/*
|    DMA1    | Stream 0         | Stream 1         | Stream 2         | Stream 3         | Stream 4         | Stream 5         | Stream 6         | Stream 7         |
|------------|------------------|------------------|------------------|------------------|------------------|------------------|------------------|------------------|
| Channel 0  | SPI3_RX_1        | -                | SPI3_RX_2        | SPI2_RX          | SPI2_TX          | SPI3_TX_1        | -                | SPI3_TX_2        |
| Channel 1  | I2C1_RX          | -                | TIM7_UP_1        |  -               | TIM7_UP_2        | I2C1_RX_1        | I2C1_TX          | I2C1_TX_1        |
| Channel 2  | TIM4_CH1         | -                | I2C4_RX          | TIM4_CH2         | -                | I2C4_RX          | TIM4_UP          | TIM4_CH3         |
| Channel 3  | -                | TIM2_UP_1        | I2C3_RX_1        | I2C2_EXT_RX      | I2C3_TX          | TIM2_CH1         | TIM2_CH2         | TIM2_UP_2        |
|            |                  |   TIM2_CH3       |                  |                  |                  |                  |   TIM2_CH4_1     |   TIM2_CH4_2     |
| Channel 4  | UART5_RX         | USART3_RX        | UART4_RX         | USART3_TX_1      | UART4_TX         | USART2_RX        | USART2_TX        | UART5_TX         |
| Channel 5  | UART8_TX         | UART7_TX         | TIM3_CH4         | UART7_RX         | TIM3_CH1         | TIM3_CH2         | UART8_RX         | TIM3_CH3         |
|            |                  |                  |   TIM3_UP        |                  |   TIM3_TRIG      |                  |                  |                  |
| Channel 6  | TIM5_CH3         | TIM5_CH4_1       | TIM5_CH1         | TIM5_CH4_2       | TIM5_CH2         | -                | TIM5_UP_2        | -                |
|            |   TIM5_UP_1      |   TIM5_TRIG_1    |                  |   TIM5_TRIG_2    |                  |                  |                  |                  |
| Channel 7  | -                | TIM6_UP          | I2C2_RX          | I2C2_RX_1        | USART3_TX_2      | DAC1             | DAC2             | I2C2_TX          |
|            |                  |                  |                  |                  |                  |                  |                  |                  |
| Usage      |                  | USART3_RX        | UART4_RX         | UART7_RX         | USART3_TX_2      | USART2_RX        | TIM4_UP          |                  |


|    DMA2    | Stream 0         | Stream 1         | Stream 2         | Stream 3         | Stream 4         | Stream 5         | Stream 6         | Stream 7         |
|------------|------------------|------------------|------------------|------------------|------------------|------------------|------------------|------------------|
| Channel 0  | ADC1_1           | SAI1_A           | TIM8_CH1_1       | SAI1_A_1         | ADC1_2           | SAI1_B_1         | TIM1_CH1_1       | -                |
|            |                  |                  |   TIM8_CH2_1     |                  |                  |                  |   TIM1_CH2_1     |                  |
|            |                  |                  |   TIM8_CH3_1     |                  |                  |                  |   TIM1_CH3_1     |                  |
| Channel 1  | -                | DCMI_1           | ADC2_1           | ADC2_2           | SAI1_B           | SPI6_TX          | SPI6_RX          | DCMI_2           |
| Channel 2  | ADC3_1           | ADC3_2           | -                | SPI5_RX_1        | SPI5_TX_1        | CRYP_OUT         | CRYP_IN          | HASH_IN          |
| Channel 3  | SPI1_RX_1        | -                | SPI1_RX_2        | SPI1_TX_1        | -                | SPI1_TX_2        | -                | QUADSPI          |
| Channel 4  | SPI4_RX_1        | SPI4_TX_1        | USART1_RX_1      | SDIO             | -                | USART1_RX_2      | SDIO             | USART1_TX        |
| Channel 5  | -                | USART6_RX_1      | USART6_RX_2      | SPI4_RX_2        | SPI4_TX_2        | -                | USART6_TX_1      | USART6_TX_2      |
| Channel 6  | TIM1_TRIG_1      | TIM1_CH1_2       | TIM1_CH2_2       | TIM1_CH1         | TIM1_CH4         | TIM1_UP          | TIM1_CH3_2       | -                |
|            |                  |                  |                  |                  |   TIM1_TRIG_2    |                  |                  |                  |
|            |                  |                  |                  |                  |   TIM1_COM       |                  |                  |                  |
| Channel 7  | -                | TIM8_UP          | TIM8_CH1_2       | TIM8_CH2_2       | TIM8_CH3_2       | SPI5_RX_2        | SPI5_TX_2        | TIM8_CH4         |
|            |                  |                  |                  |                  |                  |                  |                  |   TIM8_TRIG      |
|            |                  |                  |                  |                  |                  |                  |                  |   TIM8_COM       |
|            |                  |                  |                  |                  |                  |                  |                  |                  |
| Usage      | SPI4_RX_1        | USART6_RX_1      | SPI1_RX_2        | SPI1_TX_1        | SPI4_TX_2        | TIM1_UP          | SDIO             | USART6_TX_2      |
 */

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

