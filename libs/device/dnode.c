#include <device/dnode.h>
#include <string.h>

#if defined(CONFIG_BOARD_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <task.h>
#endif

#include <board_config.h>

#ifndef CONFIG_DRIVER_DEVICE_NODE_NUM
#define CONFIG_DRIVER_DEVICE_NODE_NUM    (20)
#endif

static uint16_t _dev_len = CONFIG_DRIVER_DEVICE_NODE_NUM;

static struct dnode _dev_list[CONFIG_DRIVER_DEVICE_NODE_NUM];

bool dn_register(const char *name, void *dev)
{
    int i = 0;
    int j = 0;
    int sz = strlen(name);
    int msz = sz < 16 ? sz : 16;

    for (; i < _dev_len; i++) {
        if (_dev_list[i]._devops == NULL) {
            _dev_list[i]._devops = dev;
            for (j = 0; j < msz; j++) {
                _dev_list[i]._devname[j] = name[j];
            }
            if (j < 16) {
                _dev_list[i]._devname[j] = '\0';
            }
            break;
        }
    }

    return (i != _dev_len);
}

void *dn_bind(const char *name)
{
    int i = 0;
    for (; i < _dev_len; i++) {
        if (_dev_list[i]._devops != NULL) {
            if (!strcmp(_dev_list[i]._devname, name)) {
                return _dev_list[i]._devops;
            }
        }
    }
    return NULL;
}

uint32_t dn_timems()
{
    uint32_t tms;
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)
    tms = xTaskGetTickCount() * portTICK_PERIOD_MS;
#else

#if defined (USE_HAL_DRIVER)
    tms = HAL_GetTick();
#else 
    tms = 0;
#endif

#endif
    return tms;
}

void dn_disable_irq()
{
    __disable_irq();
}

void dn_enable_irq()
{
    __enable_irq();
}