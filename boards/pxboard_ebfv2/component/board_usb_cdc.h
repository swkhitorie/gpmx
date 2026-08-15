#ifndef BOARD_USB_CDC_H_
#define BOARD_USB_CDC_H_

#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usbd_cdc_acm.h"
#include "usb_config.h"

#ifdef __cplusplus
extern "C" {
#endif

void board_cdc_acm_init(uint8_t busid, uintptr_t reg_base);
int  board_cdc_acm_read(uint8_t busid, uint8_t *p, uint16_t len);
int  board_cdc_acm_send(uint8_t busid, const uint8_t *p, uint16_t len, uint8_t way);

#ifdef __cplusplus
}
#endif

#endif
