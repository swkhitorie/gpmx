#ifndef DRIVE_CAN_H_
#define DRIVE_CAN_H_

#include "drv_cmn.h"
#include "drv_gpio.h"
#include <device/can.h>

struct up_can_dev_s
{
    struct can_dev_s dev;

    uint8_t id;
    struct periph_pin_t txpin;
    struct periph_pin_t rxpin;
    uint8_t priority;

    CAN_HandleTypeDef hcan;
    CAN_FilterTypeDef hfilter;
};

extern const struct can_ops_s g_can_master_ops;

#endif
