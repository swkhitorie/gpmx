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
