#include "sch1633_hw.h"


// void hw_init(void)
// {
//     // GPIO_InitTypeDef GPIO_InitStruct;
//     // __HAL_RCC_GPIOA_CLK_ENABLE();
//     // __HAL_RCC_GPIOB_CLK_ENABLE();
//     // __HAL_RCC_GPIOC_CLK_ENABLE();

//     // GPIO_InitStruct.Pin   = EXTRESN_PIN;
//     // GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
//     // GPIO_InitStruct.Pull  = GPIO_PULLUP;
//     // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//     // HAL_GPIO_Init(EXTRESN_PORT, &GPIO_InitStruct);
//     // // Set EXTRESN inactive (high)
//     // HAL_GPIO_WritePin(EXTRESN_PORT, EXTRESN_PIN, GPIO_PIN_SET);

//     // HAL_Delay(100);
// }

// void hw_EXTRESN_High(void)
// {
// 	// HAL_GPIO_WritePin(EXTRESN_PORT, EXTRESN_PIN, GPIO_PIN_SET);
// }

// void hw_EXTRESN_Low(void)
// {
// 	// HAL_GPIO_WritePin(EXTRESN_PORT, EXTRESN_PIN, GPIO_PIN_RESET);
// }

// void hw_CS_High()
// {
// 	// HAL_GPIO_WritePin(SPI2_CS_PORT, SPI2_CS_PIN, GPIO_PIN_SET);
// }
// void hw_CS_Low()
// {
// 	// HAL_GPIO_WritePin(SPI2_CS_PORT, SPI2_CS_PIN, GPIO_PIN_RESET);
// }

// void hw_delay(uint32_t ms)
// {
// 	// HAL_Delay(ms);
// }

// uint64_t hw_SPI48_Send_Request(uint64_t Request)
// {
//     // uint64_t ReceivedData = 0;
//     // uint16_t txBuffer[3];
//     // uint16_t rxBuffer[3];
//     // uint8_t index;
//     // uint8_t size = 3;   // 48-bit SPI-transfer consists of three 16-bit transfers.

//     // // Split Request qword (MOSI data) to tx buffer.
//     // for (index = 0; index < size; index++)
//     // {
//     //     txBuffer[size - index - 1] = (Request >> (index << 4)) & 0xFFFF;
//     // }

//     // //printf("[spi] start: %llx \r\n", Request);
//     // // Send tx buffer and receive rx buffer simultaneously.
//     // hw_CS_Low();
//     // HAL_SPI_TransmitReceive(&hspi2, (uint8_t*)txBuffer, (uint8_t*)rxBuffer, size, 3000);
//     // hw_CS_High();

//     // // Create ReceivedData qword from received rx buffer (MISO data).
//     // for (index = 0; index < size; index++)
//     // {
//     // 	ReceivedData |= (uint64_t)rxBuffer[index] << ((size - index - 1) << 4);
//     // }

//     // //printf("[spi] recv: %llx \r\n", ReceivedData);
//     // return ReceivedData;
// }


#if (SCH1633_ADAPT_PLATFORM == 1)

#include <device/spi.h>
#include <drv_gpio.h>
#include <drv_spi.h>
#include <board_config.h>

static struct spi_dev_s *sbus;
void hw_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    sbus = spi_bus_get(1);
    if (!sbus) {
        printf("[spi] spidev1 get failed \r\n");
    }

    __HAL_RCC_GPIOE_CLK_ENABLE();
   // LOW_INITPIN(GPIOA, 3, IOMODE_OUTPP, IO_PULLUP, IO_SPEEDHIGH);
    GPIO_InitStruct.Pin   = GPIO_PIN_3;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    // Set EXTRESN inactive (high)
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);

    hw_EXTRESN_High();

    HAL_Delay(100);
}

void hw_EXTRESN_High(void)
{
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
    // LOW_IOSET(GPIOA, 3, 1);
}

void hw_EXTRESN_Low(void)
{
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
    // LOW_IOSET(GPIOA, 3, 0);
}

void hw_CS_High()
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
    // LOW_IOSET(GPIOA, 15, 1);
}
void hw_CS_Low()
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
    // LOW_IOSET(GPIOA, 15, 0);
}

void hw_delay(uint32_t ms)
{
	HAL_Delay(ms);
}

uint64_t hw_SPI48_Send_Request(uint64_t Request)
{
    uint64_t ReceivedData = 0;
    uint16_t txBuffer[3];
    uint16_t rxBuffer[3];
    uint8_t index;
    uint8_t size = 3;   // 48-bit SPI-transfer consists of three 16-bit transfers.

    // Split Request qword (MOSI data) to tx buffer.
    for (index = 0; index < size; index++)
    {
        txBuffer[size - index - 1] = (Request >> (index << 4)) & 0xFFFF;
    }

    //printf("[spi] start: %llx \r\n", Request);
    // Send tx buffer and receive rx buffer simultaneously.
    hw_CS_Low();
    //SPI_EXCHANGEBLOCK(sbus, (uint8_t*)txBuffer, (uint8_t*)rxBuffer, size);
    struct up_spi_dev_s *priv = (struct up_spi_dev_s *)sbus->priv;
    HAL_SPI_TransmitReceive(&priv->hspi, (uint8_t*)txBuffer, (uint8_t*)rxBuffer, size, 3000);
    hw_CS_High();

    // Create ReceivedData qword from received rx buffer (MISO data).
    for (index = 0; index < size; index++)
    {
        ReceivedData |= (uint64_t)rxBuffer[index] << ((size - index - 1) << 4);
    }

    // printf("[spi] recv: %lx \r\n", ReceivedData);
    return ReceivedData;
}

#endif
