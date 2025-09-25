#include <FreeRTOS.h>
#include <task.h>

#include <board_config.h>
#include <drivers/drv_hrt.h>
#include <stdio.h>
#include <stdint.h>

#include "sensor_task.h"
#include "upgrade_task.h"

void g_start(void *p)
{
    board_bsp_init();
    hrt_init();

    xTaskCreate(sensors_task, "sensor", 512, NULL, 6, NULL);
    xTaskCreate(upgrade_task, "upgrade", 256, NULL, 1, NULL);

    vTaskDelete(NULL);
}

int main(int argc, char *argv[])
{
    board_init();

    taskENTER_CRITICAL(); 
    xTaskCreate(g_start, "g_start", 256, NULL, 1, NULL);
    taskEXIT_CRITICAL();

    vTaskStartScheduler();
    for (;;);
}
