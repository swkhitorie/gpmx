#include "sensor_task.h"

#include <board_config.h>
#include <drivers/drv_hrt.h>
#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include "tca62724.h"

void sensors_task(void *p)
{
    uint32_t ticks = 0;
    uint8_t rgb_toggle = 0;
    uint32_t lst_wakeup = xTaskGetTickCount();

    rgb_tca62724_init(2);

    for (;;) {

        vTaskDelayUntil(&lst_wakeup, pdMS_TO_TICKS(1));

        if ((ticks % 100) == 0) {
            rgb_toggle = !rgb_toggle;
            if (rgb_toggle == 0) {
                rgb_tca62724_set(RGB_COLOR_PURPLE, 50);
            } else {
                rgb_tca62724_set(RGB_COLOR_NONE, 0);
            }

            board_debug();
            printf("sensor task debug\r\n");
        }

        ticks++;
    }
}
