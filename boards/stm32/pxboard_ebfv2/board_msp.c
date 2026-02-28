#include <board_config.h>
#include <drv_gpio.h>
#include <drv_uart.h>
#include <drv_spi.h>
#include <drv_mmcsd.h>

void DMA2_Stream7_IRQHandler()
{
    STM32_USART_DMA_TX_IRQ(1);
}

void DMA2_Stream2_IRQHandler()
{
    STM32_USART_DMA_RX_IRQ(1);
}

// void DMA2_Stream3_IRQHandler()
// {
//     STM32_SPI_DMA_TX_IRQ(1);
// }

// void DMA2_Stream2_IRQHandler()
// {
//     STM32_SPI_DMA_RX_IRQ(1);
// }

void DMA1_Stream3_IRQHandler()
{
    STM32_USART_DMA_TX_IRQ(3);
}

void DMA1_Stream1_IRQHandler()
{
    STM32_USART_DMA_RX_IRQ(3);
}

void DMA1_Stream0_IRQHandler()
{
    STM32_USART_DMA_RX_IRQ(5);
}

void DMA2_Stream1_IRQHandler()
{
    STM32_USART_DMA_RX_IRQ(6);
}

void HAL_ETH_MspInit(ETH_HandleTypeDef* ethHandle)
{
    /** 
     * ETH RMII IO Config    
     * PC1     ------> ETH_MDC
     * PA1     ------> ETH_REF_CLK
     * PA2     ------> ETH_MDIO
     * PA7     ------> ETH_CRS_DV
     * PC4     ------> ETH_RXD0
     * PC5     ------> ETH_RXD1
     * PG11     ------> ETH_TX_EN
     * PG13     ------> ETH_TXD0
     * PG14     ------> ETH_TXD1 
    */

    /** ETH_MDC, ETH_RXD0, ETH_RXD1 */
    LOW_PERIPH_INITPIN(GPIOC, 1, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, GPIO_AF11_ETH);
    LOW_PERIPH_INITPIN(GPIOC, 4, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, GPIO_AF11_ETH);
    LOW_PERIPH_INITPIN(GPIOC, 5, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, GPIO_AF11_ETH);

    /** ETH_REF_CLK, ETH_MDIO, ETH_CRS_DV */
    LOW_PERIPH_INITPIN(GPIOA, 1, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, GPIO_AF11_ETH);
    LOW_PERIPH_INITPIN(GPIOA, 2, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, GPIO_AF11_ETH);
    LOW_PERIPH_INITPIN(GPIOA, 7, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, GPIO_AF11_ETH);

    /** ETH_TX_EN */
    LOW_PERIPH_INITPIN(GPIOG, 11, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, GPIO_AF11_ETH);

    /** ETH_TXD0, ETH_TXD1 */
    LOW_PERIPH_INITPIN(GPIOG, 13, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, GPIO_AF11_ETH);
    LOW_PERIPH_INITPIN(GPIOG, 14, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, GPIO_AF11_ETH);

    /** PHY Reset Pin Config */
    LOW_INITPIN(GPIOI, 1, IOMODE_OUTPP, IO_PULLUP, IO_SPEEDHIGH);

}

void phy_reset()
{
    // pin low
    LOW_IOSET(GPIOI, 1, 0);
    HAL_Delay(5);
    LOW_IOSET(GPIOI, 1, 1);
    HAL_Delay(5);
}

