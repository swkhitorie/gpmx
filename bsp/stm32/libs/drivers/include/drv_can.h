#ifndef DRV_CAN_H_
#define DRV_CAN_H_

#include "drv_common.h"
#include "drv_gpio.h"
#if defined (DRV_BSP_H7)
#include "drv_pin_h7.h"
#endif

struct low_canframe;
struct low_canfifo;
struct up_can_dev_s
{
    uint32_t baud;

    uint8_t id;
    uint8_t pin_tx;
    uint8_t pin_rx;
    uint8_t priority;
    struct low_canfifo fifo;

    FDCAN_HandleTypeDef hcan;
};

#ifdef __cplusplus
extern "C" {
#endif

void low_can_init(struct up_can_dev_s *dev);

void low_can_send(struct up_can_dev_s *dev, struct low_canframe *frame);

bool low_can_recv(struct up_can_dev_s *dev, struct low_canframe *frame);

#ifdef __cplusplus
}
#endif

#endif
