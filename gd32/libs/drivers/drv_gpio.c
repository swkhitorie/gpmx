#include "drv_gpio.h"

struct drv_pin_t drv_gpio_init(uint32_t port, uint32_t pin, uint32_t mode, uint32_t speed)
{
    struct drv_pin_t obj;
    rcu_periph_enum clk;
    uint32_t io_pin = (0x01 << pin);
    if (port == GPIOA) {
        clk = RCU_GPIOA;
    } else if (port == GPIOB) {
        clk = RCU_GPIOB;
    } else if (port == GPIOC) {
        clk = RCU_GPIOC;
    } else if (port == GPIOD) {
        clk = RCU_GPIOD;
    } else if (port == GPIOE) {
        clk = RCU_GPIOE;
    }
    rcu_periph_clock_enable(clk);
    gpio_init(port, mode, speed, io_pin);
    
    obj.port = port;
    obj.pin = io_pin;
    return obj;
}

void drv_gpio_deinit(struct drv_pin_t *obj)
{
    gpio_deinit(obj->port);
}

void drv_gpio_write(struct drv_pin_t *obj, uint8_t val)
{
    gpio_bit_write(obj->port, obj->pin, val);
}

void drv_gpio_read(struct drv_pin_t *obj)
{
    gpio_input_bit_get(obj->port, obj->pin);
}






