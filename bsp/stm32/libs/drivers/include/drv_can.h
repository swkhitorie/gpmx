#ifndef DRV_CAN_H_
#define DRV_CAN_H_

#include "drv_common.h"
#include "drv_gpio.h"
#if defined (DRV_BSP_H7)
#include "drv_pin_h7.h"
#endif

struct drv_canframe {
    uint32_t _identifier;
    uint32_t _idtype;
    uint32_t _frametype;
    uint32_t _datalength;
    uint8_t _data[8];
};

struct drv_can_attr {
    uint32_t baud;
    uint16_t priority;
    struct drv_canframe *array;
    uint16_t capacity;
    uint16_t size;
    uint16_t in;
    uint16_t out;
};

struct drc_can_t {
    FDCAN_HandleTypeDef hcan;
    uint8_t cannum;
    struct drv_can_attr attr;
}

extern drc_can_t *drc_can_list[2];

#ifdef __cplusplus
extern "C" {
#endif

/**	
 * 	Initialize CAN Bus, include GPIO, CAN, CAN Filter, and NVIC. 
 *	the can bus baudrate computational formula is --->
 *						can bus clockFreq				200000000 (200MHz)
 *	can bus baudrate  = -------------------------- = ------------------------ = (500KHz)
 *						(ntsg1 + ntsg2 + 1) * presc 	(31 + 8 + 1) * 10
*/

void drv_can_attr_init(struct drv_can_attr *obj, uint32_t baud, uint16_t priority,
                    struct drv_canframe *array, uint16_t len);

void drv_can_init(struct drc_can_t *obj, uint8_t num, uint8_t tx_selec, 
                    uint8_t rx_selec, struct drv_can_attr *attr);

void drv_can_send(struct drc_can_t *obj, struct drv_canframe *frame);

bool drv_can_get(struct drc_can_t *obj, struct drv_canframe *frame);

void drv_can_rx_irq(struct drc_can_t *obj, uint32_t fifox);

#ifdef __cplusplus
}
#endif

#endif
