#ifndef DRV_GPIO_H_
#define DRV_GPIO_H_

#include <stdint.h>
#include "drv_common.h"

#define DRV_GPIO_EXTERNAL_IRQ_LINE_NUM CONFIG_DRV_GPIO_EXTERNAL_IRQ_LINE_NUM

struct drv_pin_irq_t {
    uint32_t priority;
    void (*entry)();
};

struct drv_pin_t {
    struct drv_pin_irq_t pin_irq_attr;
    GPIO_TypeDef *port;   
    uint16_t pin;
    uint32_t alternate;
};

enum pullstate {
    IO_NOPULL = GPIO_NOPULL,
    IO_PULLUP = GPIO_PULLUP,
    IO_PULLDOWN = GPIO_PULLDOWN,
};

enum iospeed {
    IO_SPEEDLOW = GPIO_SPEED_FREQ_LOW,
    IO_SPEEDMID = GPIO_SPEED_FREQ_MEDIUM,
    IO_SPEEDHIGH = GPIO_SPEED_FREQ_HIGH,
#ifndef DRV_F1
    IO_SPEEDMAX = GPIO_SPEED_FREQ_VERY_HIGH,
#endif
};

enum iomode {
    IOMODE_INPUT = GPIO_MODE_INPUT,
    IOMODE_OUTPP = GPIO_MODE_OUTPUT_PP,
    IOMODE_OUTOD = GPIO_MODE_OUTPUT_OD,
    IOMODE_AFPP = GPIO_MODE_AF_PP,
    IOMODE_AFOD = GPIO_MODE_AF_OD,
    IOMODE_ANALOG = GPIO_MODE_ANALOG,
    IOMODE_IT_RISING = GPIO_MODE_IT_RISING,
    IOMODE_IT_FALLING = GPIO_MODE_IT_FALLING,
    IOMODE_IT_BOTH = GPIO_MODE_IT_RISING_FALLING,
};

extern struct drv_pin_t *drv_external_irq_pin_list[GPIO_EXTERNAL_IRQ_LINE_NUM];

#ifdef cplusplus
  extern "C" {
#endif

struct drv_pin_t drv_gpio_init(GPIO_TypeDef *port, uint32_t pin, uint32_t mode, 
                    uint32_t pull, uint32_t speed, uint32_t alternate, struct drv_pin_irq_t *irq);
void drv_gpio_deinit(struct drv_pin_t *obj);
void drv_gpio_write(struct drv_pin_t *obj, uint8_t val);
uint8_t drv_gpio_read(struct drv_pin_t *obj);


#ifdef cplusplus
}
#endif

#endif
