#ifndef __SX126X_HAL_H__
#define __SX126X_HAL_H__

#include <stdint.h>
#include <stdbool.h>

/**
 * Platform Drivers
 */
#include <board_config.h>
#define SX126X_MS_DELAY         HAL_Delay
#define SX126X_TIMESTAMP        HAL_GetTick
#define SX126X_RX_BUFFER_LEN    (300)


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Init SPI Mode: 8bytes, MODE0(Polarity:low, Edge:first)
 * 
 * RESET Pin: Output, Pull UP, LOW Active
 * BUSY Pin:  Input,  No Pull, High Level when device in busy
 * DIO1 Pin:  Input,  Pull Down, Rising Irq 
 * DIO2 Pin: ???
 * SWCTL1 Pin: ???
 * SWCTL2 Pin: ???
 */
void sx126x_hal_init();

void sx126x_hal_nss_set(uint8_t state);

void sx126x_hal_reset_set(uint8_t state);

void sx126x_hal_swctl_1_set(uint8_t state);

void sx126x_hal_swctl_2_set(uint8_t state);

void sx126x_hal_wait_onbusy();

uint8_t sx126x_hal_exchange_byte(uint8_t sndval);

/***
 * sx126x running irq
 */
void sx126x_irq();


#ifdef __cplusplus
}
#endif



#endif
