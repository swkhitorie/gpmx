#ifndef DRV_CAN_H_
#define DRV_CAN_H_

#include "drv_common.h"
#include "drv_gpio.h"
#if defined (DRV_BSP_H7)
#include "drv_pin_h7.h"
#endif
#include <device/can.h>

struct stm32_baud_rate_tab
{
    uint32_t baud;
    uint32_t config;
};

#define BS1SHIFT        16
#define BS2SHIFT        20
#define RRESCLSHIFT     0
#define SJWSHIFT        24

#define BS1MASK         ((0x0F) << BS1SHIFT)
#define BS2MASK         ((0x07) << BS2SHIFT)
#define RRESCLMASK      ((0x3FF) << RRESCLSHIFT)
#define SJWMASK         ((0x3) << SJWSHIFT)

#define CAN_EVENT_RX_IND     (0x0010)
#define CAN_EVENT_RXOF_IND   (0x0011)
#define CAN_EVENT_TX_FAIL    (0x0021)
#define CAN_EVENT_TX_DONE    (0x0022)

struct up_can_dev_s
{
    struct can_dev_s dev;

    uint8_t id;
    uint8_t pin_tx;
    uint8_t pin_rx;
    uint8_t priority;

    CAN_HandleTypeDef hcan;
    CAN_FilterTypeDef hfilter;
};

extern const struct can_ops_s g_can_master_ops;

#endif
