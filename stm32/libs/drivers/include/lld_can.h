#ifndef __LLD_CAN_H_
#define __LLD_CAN_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * low level driver for stm32h7 series, base on cubehal library
 * module can
*/

#include "lld_h7_global.h"
#include "lld_gpio.h"

#define LLD_CANFRAMELIST_LEN    (20)
    
typedef struct __lld_canframe {
    uint32_t _identifier;
    uint32_t _idtype;
    uint32_t _frametype;
    uint32_t _datalength;
    uint8_t _data[8];
} lld_canframe_t;

typedef struct __lld_canframelist {
    lld_canframe_t buf[LLD_CANFRAMELIST_LEN];
    uint8_t size;
    uint8_t idx_in;
    uint8_t idx_out;
} lld_canframelist_t;

typedef struct __lld_can {
    FDCAN_HandleTypeDef hcan;
    uint8_t cannum;
    uint32_t baud;
    lld_gpio_t txpin;
    lld_gpio_t rxpin;
    lld_canframelist_t rxlist;
} lld_can_t;
    
extern lld_can_t *mcu_can_list[2];

/**
 * @brief 	
 * 			 Initialize CAN Bus, include GPIO, CAN, CAN Filter, and NVIC. 
 *			 the can bus baudrate computational formula is --->
 *									can bus clockFreq					200000000 (200MHz)
 *			can bus baudrate  = ---------------------------------- = ------------------------ = (500KHz)
 *									(ntsg1 + ntsg2 + 1) * presc 		(31 + 8 + 1) * 10
*/
void lld_can_init(lld_can_t *obj, uint8_t can, uint32_t baud, uint8_t tx_selec, uint8_t rx_selec);
void lld_can_sendframe(lld_can_t *obj, uint32_t id, uint32_t idtype, uint32_t frametype, uint8_t *data, uint8_t len);
void lld_can_rx_irq(lld_can_t *obj, uint32_t fifox);

void lld_canframe_set(lld_canframe_t *obj, uint32_t id, uint32_t idtype, uint32_t frametype, uint32_t len, uint8_t *data);
void lld_canframelist_reset(lld_canframelist_t *obj);
uint8_t lld_canframelist_size(lld_canframelist_t *obj);
void lld_canframelist_read(lld_canframelist_t *obj, lld_canframe_t *p);
void lld_canframelist_write(lld_canframelist_t *obj, lld_canframe_t *p);


#ifdef __cplusplus
}
#endif

#endif
