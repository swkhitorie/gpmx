#ifndef DRV_UART_H_
#define DRV_UART_H_

#include <stdint.h>
#include "drv_common.h"
#include "drv_gpio.h"
#if defined (DRV_BSP_H7)
#include "drv_pin_h7.h"
#endif
#include <device/serial.h>

#define DRV_UART_PERIPHAL_NUM   CONFIG_UART_PERIPHAL_NUM

struct up_uart_dev_s
{
    struct uart_dev_s dev;   /* Generic UART device */

    uint8_t id;
    uint8_t pin_tx;
    uint8_t pin_rx;
    uint8_t priority;
    uint8_t priority_dmatx;
    uint8_t priority_dmarx;
    bool enable_dmatx;
    bool enable_dmarx;

    UART_HandleTypeDef com;
    DMA_HandleTypeDef txdma;
    DMA_HandleTypeDef rxdma;
};

extern const struct uart_ops_s g_uart_ops;

extern struct uart_dev_s *g_uart_list[DRV_UART_PERIPHAL_NUM];

#endif

