#include <board_config.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

void debug1(void *p)
{
    int cnt = 1;
    for (;;) {
        char tmp1[] = "=================== debug1 : send=================== \r\n";
        printf("%s", tmp1);
        vTaskDelay(1000);
    }
}

void debug2(void *p)
{
    for (;;) {
        char tmp1[] = "=================== debug2 : send=================== \r\n";
        printf("%s", tmp1);
        vTaskDelay(10);
    }
}

void os_start(void *p)
{
    board_bsp_init();

    taskENTER_CRITICAL(); 
    xTaskCreate(debug1, "debug1", 256, NULL, 3, NULL);
    xTaskCreate(debug2, "debug2", 256, NULL, 4, NULL);
    taskEXIT_CRITICAL();

    vTaskDelete(NULL);
}

int main(int argc, char *argv[])
{
    board_init();

    taskENTER_CRITICAL(); 
    xTaskCreate(os_start, "start", 256, NULL, 1, NULL);
    taskEXIT_CRITICAL();

    vTaskStartScheduler();
    for (;;);
}
