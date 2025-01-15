#ifndef CDC_ACM_USER_H_
#define CDC_ACM_USER_H_

#ifdef __cplusplus
    extern "C" {
#endif

#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usbd_cdc_acm.h"
#include "usb_config.h"

void cdc_acm_init(uint8_t busid, uintptr_t reg_base);
void cdc_acm_data_send_with_dtr_test(uint8_t busid);
void cdc_acm_print(uint8_t busid, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
