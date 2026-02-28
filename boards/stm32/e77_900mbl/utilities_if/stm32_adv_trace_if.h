#ifndef __STM32_ADV_TRACE_IF_H__
#define __STM32_ADV_TRACE_IF_H__

#include "stm32_adv_trace.h"

#ifdef __cplusplus
extern "C" {
#endif

UTIL_ADV_TRACE_Status_t UART_Init(void (*cb)(void *));
UTIL_ADV_TRACE_Status_t UART_DeInit(void);
UTIL_ADV_TRACE_Status_t UART_TransmitDMA(uint8_t *pdata, uint16_t size);
UTIL_ADV_TRACE_Status_t UART_StartRx(void (*cb)(uint8_t *pdata, uint16_t size, uint8_t error));

#ifdef __cplusplus
}
#endif

#endif
