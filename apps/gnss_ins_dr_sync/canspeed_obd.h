#ifndef __CAN_OBD_H_
#define __CAN_OBD_H_

#include <stdint.h>
#include <board_config.h>

#ifdef __cplusplus
extern "C" {
#endif

void obd_bus_init();

void obd_request_speed();

void obd_rx_speed_detect();

uint32_t obd_read_speed();

#ifdef __cplusplus
}
#endif


#endif
