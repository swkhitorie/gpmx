#include "stm32_adv_trace.h"
#include "stm32_adv_trace_if.h"

const UTIL_ADV_TRACE_Driver_s UTIL_TraceDriver =
{
  UART_Init,
  UART_DeInit,
  UART_StartRx,
  UART_TransmitDMA
};

UTIL_ADV_TRACE_Status_t UART_Init(void (*cb)(void *))
{
  return UTIL_ADV_TRACE_OK;
}

UTIL_ADV_TRACE_Status_t UART_DeInit(void)
{
  return UTIL_ADV_TRACE_OK;
}

UTIL_ADV_TRACE_Status_t UART_StartRx(void (*cb)(uint8_t *pdata, uint16_t size, uint8_t error))
{
  return UTIL_ADV_TRACE_OK;
}

UTIL_ADV_TRACE_Status_t UART_TransmitDMA(uint8_t *pdata, uint16_t size)
{
  return UTIL_ADV_TRACE_OK;
}

