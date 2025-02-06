#ifndef DEV_CDC_ACM_H_
#define DEV_CDC_ACM_H_

#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usbd_cdc_acm.h"
#include "usb_config.h"

#ifdef __cplusplus
extern "C" {
#endif

void cdc_acm_init(uint8_t busid, uintptr_t reg_base);
void dev_cdc_acm_init(uint8_t busid, uintptr_t reg_base);

int dev_cdc_acm_send(uint8_t busid, const uint8_t *p, uint16_t len);

int dev_cdc_acm_rsize(uint8_t busid);

int dev_cdc_acm_read(uint8_t busid, uint8_t *p, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
