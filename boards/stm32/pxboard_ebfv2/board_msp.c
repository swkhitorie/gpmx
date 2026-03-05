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

#if !defined(CONFIG_GPDRIVE_MMCSDSPI)
static DMA_HandleTypeDef dmaRxHandle;
static DMA_HandleTypeDef dmaTxHandle;
void HAL_SD_MspInit(SD_HandleTypeDef *hsd)
{
    /**
     * MMCSD 
     *  CLK - PC12
     *  CMD - PD2
     *  D0 - PC8
     *  D1 - PC9
     *  D2 - PC10
     *  D3 - PC11
     */
    LOW_PERIPH_INITPIN(GPIOC, 12, IOMODE_AFPP, IO_PULLUP, IO_SPEEDHIGH, GPIO_AF12_SDIO);  
    LOW_PERIPH_INITPIN(GPIOD, 2,  IOMODE_AFPP, IO_PULLUP, IO_SPEEDHIGH, GPIO_AF12_SDIO);

    LOW_PERIPH_INITPIN(GPIOC, 8,  IOMODE_AFPP, IO_PULLUP, IO_SPEEDHIGH, GPIO_AF12_SDIO);
    LOW_PERIPH_INITPIN(GPIOC, 9,  IOMODE_AFPP, IO_PULLUP, IO_SPEEDHIGH, GPIO_AF12_SDIO);
    LOW_PERIPH_INITPIN(GPIOC, 10, IOMODE_AFPP, IO_PULLUP, IO_SPEEDHIGH, GPIO_AF12_SDIO);
    LOW_PERIPH_INITPIN(GPIOC, 11, IOMODE_AFPP, IO_PULLUP, IO_SPEEDHIGH, GPIO_AF12_SDIO);

    __HAL_RCC_DMA2_CLK_ENABLE();
    dmaRxHandle.Init.Channel             = DMA_CHANNEL_4;
    dmaRxHandle.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    dmaRxHandle.Init.PeriphInc           = DMA_PINC_DISABLE;
    dmaRxHandle.Init.MemInc              = DMA_MINC_ENABLE;
    dmaRxHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    dmaRxHandle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    dmaRxHandle.Init.Mode                = DMA_PFCTRL;
    dmaRxHandle.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    dmaRxHandle.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
    dmaRxHandle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    dmaRxHandle.Init.MemBurst            = DMA_MBURST_INC4;
    dmaRxHandle.Init.PeriphBurst         = DMA_PBURST_INC4;
    dmaRxHandle.Instance = DMA2_Stream3;
    __HAL_LINKDMA(hsd, hdmarx, dmaRxHandle);
    HAL_DMA_DeInit(&dmaRxHandle);
    HAL_DMA_Init(&dmaRxHandle);

    dmaTxHandle.Init.Channel             = DMA_CHANNEL_4;
    dmaTxHandle.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    dmaTxHandle.Init.PeriphInc           = DMA_PINC_DISABLE;
    dmaTxHandle.Init.MemInc              = DMA_MINC_ENABLE;
    dmaTxHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    dmaTxHandle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    dmaTxHandle.Init.Mode                = DMA_PFCTRL;
    dmaTxHandle.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    dmaTxHandle.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
    dmaTxHandle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    dmaTxHandle.Init.MemBurst            = DMA_MBURST_INC4;
    dmaTxHandle.Init.PeriphBurst         = DMA_PBURST_INC4;
    dmaTxHandle.Instance = DMA2_Stream6;
    __HAL_LINKDMA(hsd, hdmatx, dmaTxHandle);
    HAL_DMA_DeInit(&dmaTxHandle);
    HAL_DMA_Init(&dmaTxHandle); 

    HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

    HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);
}

void DMA2_Stream3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&dmaRxHandle);
}

void DMA2_Stream6_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&dmaTxHandle); 
}
#endif

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

