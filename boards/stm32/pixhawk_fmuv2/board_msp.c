#include <board_config.h>
#include <drv_uart.h>
#include <drv_spi.h>
#include <drv_mmcsd.h>

void DMA1_Stream6_IRQHandler()
{
    STM32_USART_DMA_TX_IRQ(2);
}
void DMA1_Stream5_IRQHandler()
{
    STM32_USART_DMA_RX_IRQ(2);
}

void DMA2_Stream3_IRQHandler()
{
    STM32_SPI_DMA_TX_IRQ(1);
}
void DMA2_Stream2_IRQHandler()
{
    STM32_SPI_DMA_RX_IRQ(1);
}

void DMA1_Stream4_IRQHandler()
{
    STM32_SPI_DMA_TX_IRQ(2);
}
void DMA1_Stream3_IRQHandler()
{
    STM32_SPI_DMA_RX_IRQ(2);
}

#if !defined(CONFIG_GPDRIVE_MMCSDSPI)
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

    HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);
}

void DMA2_Stream6_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&dmaTxHandle);
}
#endif
