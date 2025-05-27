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
        vTaskDelay(2000);
    }
}

void debug2(void *p)
{
    for (;;) {
        char tmp1[] = "=================== debug2 : send=================== \r\n";
        printf("%s", tmp1);
        vTaskDelay(100);
    }
}

void debug3(void *p)
{
    for (;;) {
        char task_list_str[512];
        const char *const header = "Task         State   Priority  Stack	#\r\n************************************************\r\n";
        strcpy(task_list_str, header);
        vTaskList(task_list_str + strlen(header));
        printf("%s\r\n", task_list_str);
        vTaskDelay(10);
    }
}

void os_start(void *p)
{
    board_bsp_init();

    taskENTER_CRITICAL(); 
    xTaskCreate(debug1, "debug1", 256, NULL, 3, NULL);
    xTaskCreate(debug2, "debug2", 256, NULL, 4, NULL);
    xTaskCreate(debug3, "debug3", 1024, NULL, 5, NULL);
    taskEXIT_CRITICAL();

    vTaskDelete(NULL);
}

/**
 * recommand : single logger task, 
 *             other task write message to logger output queue
 */
int main(int argc, char *argv[])
{
    board_init();

    taskENTER_CRITICAL(); 
    xTaskCreate(os_start, "start", 256, NULL, 1, NULL);
    taskEXIT_CRITICAL();

    vTaskStartScheduler();
    for (;;);
}
