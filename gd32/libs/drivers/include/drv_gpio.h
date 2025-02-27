#ifndef DRV_GPIO_H_
#define DRV_GPIO_H_

#include <stdint.h>
#include "gd32c10x.h"

struct drv_pin_t {
    uint32_t port;
    uint32_t pin;
};

enum iomode {
    IOMODE_AIN = GPIO_MODE_AIN,
    IOMODE_IN_FLOATING = GPIO_MODE_IN_FLOATING,
    IOMODE_IPD = GPIO_MODE_IPD,
    IOMODE_IPU = GPIO_MODE_IPU,
    IOMODE_OUT_OD = GPIO_MODE_OUT_OD,
    IOMODE_OUT_PP = GPIO_MODE_OUT_PP,
    IOMODE_AF_OD = GPIO_MODE_AF_OD,
    IOMODE_AF_PP = GPIO_MODE_AF_PP
};

enum speed {
    IOSPEED_10M = GPIO_OSPEED_10MHZ,
    IOSPEED_2M = GPIO_OSPEED_2MHZ,
    IOSPEED_50M = GPIO_OSPEED_50MHZ,
    IOSPEED_MAX = GPIO_OSPEED_MAX
};

#ifdef cplusplus
  extern "C" {
#endif

struct drv_pin_t drv_gpio_init(uint32_t port, uint32_t pin, uint32_t mode, uint32_t speed);
void drv_gpio_deinit(struct drv_pin_t *obj);
void drv_gpio_write(struct drv_pin_t *obj, uint8_t val);
void drv_gpio_read(struct drv_pin_t *obj);


#ifdef cplusplus
}
#endif

#endif
