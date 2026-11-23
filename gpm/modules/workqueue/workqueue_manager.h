#ifndef WORKQUEUE_MANAGER_H_
#define WORKQUEUE_MANAGER_H_

#include <gpmx/config.h>
#include <stdint.h>

struct __workqueue;

struct wq_config_t {
    const char *name;
    uint16_t stacksize;
    int8_t relative_priority; // relative to max
};

#ifdef __cplusplus
extern "C" {
#endif

extern const struct wq_config_t wq_config_rate_ctrl;

extern const struct wq_config_t wq_config_spi0;
extern const struct wq_config_t wq_config_spi1;
extern const struct wq_config_t wq_config_spi2;
extern const struct wq_config_t wq_config_spi3;
extern const struct wq_config_t wq_config_spi4;
extern const struct wq_config_t wq_config_spi5;
extern const struct wq_config_t wq_config_spi6;

extern const struct wq_config_t wq_config_i2c0;
extern const struct wq_config_t wq_config_i2c1;
extern const struct wq_config_t wq_config_i2c2;
extern const struct wq_config_t wq_config_i2c3;
extern const struct wq_config_t wq_config_i2c4;

// PX4 att/pos controllers, highest priority after sensors.
extern const struct wq_config_t wq_config_nav_and_controllers;

extern const struct wq_config_t wq_config_ins0;
extern const struct wq_config_t wq_config_ins1;
extern const struct wq_config_t wq_config_ins2;
extern const struct wq_config_t wq_config_ins3;

extern const struct wq_config_t wq_config_hp_default;
extern const struct wq_config_t wq_config_uavcan;

extern const struct wq_config_t wq_config_uart0;
extern const struct wq_config_t wq_config_uart1;
extern const struct wq_config_t wq_config_uart2;
extern const struct wq_config_t wq_config_uart3;
extern const struct wq_config_t wq_config_uart4;
extern const struct wq_config_t wq_config_uart5;
extern const struct wq_config_t wq_config_uart6;
extern const struct wq_config_t wq_config_uart7;
extern const struct wq_config_t wq_config_uart8;
extern const struct wq_config_t wq_config_ttyacm0;
extern const struct wq_config_t wq_config_uart_unknown;

extern const struct wq_config_t wq_config_lp_default;

extern const struct wq_config_t wq_config_test1;
extern const struct wq_config_t wq_config_test2;

const struct wq_config_t *device_bus_to_wq(const char *bus);
const struct wq_config_t *serial_port_to_wq(const char *serial);
const struct wq_config_t *ins_instance_to_wq(uint8_t instance);

int workqueue_manager_start();
int workqueue_manager_stop();
int workqueue_manager_status();
struct __workqueue *workqueue_find_or_create(const struct wq_config_t *new_wq);

#ifdef __cplusplus
}
#endif


#endif
