#include "board_config.h"
#include <stdio.h>
#include <drivers/drv_hrt.h>
#include "FreeRTOS.h"
#include "task.h"

void debug1(void *p)
{
    for (;;) {
        printf("debug1 : send \r\n");
        vTaskDelay(1000);
    }
}

void debug2(void *p)
{
    for (;;) {
        printf("debug2 : send \r\n");
        vTaskDelay(10);
    }
}

int main(void)
{
    board_init();
    hrt_init();

    xTaskCreate(debug1, "debug1", 256, NULL, 3, NULL);
    xTaskCreate(debug2, "debug2", 256, NULL, 3, NULL);
    vTaskStartScheduler();
    for (;;);
}
