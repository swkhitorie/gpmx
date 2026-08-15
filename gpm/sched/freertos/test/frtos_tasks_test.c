#include "frtos_test.h"

#include "driver/mlog.h"

#include "FreeRTOS.h"
#include "task.h"

void task_1(void *p)
{
    int cnt = 1;
    for (;;) {
        KMRAW("task_1 : run \r\n");
        cnt++;
        if (cnt == 5) {
            vTaskSuspend(NULL);
        }
        vTaskDelay(100);
    }
}

static char pstr1[1024];
static char pstr2[1024];
void task_2(void *p)
{
    for (;;) {

#if ( configGENERATE_RUN_TIME_STATS == 1 )
        vTaskGetRunTimeStats(pstr1);
#endif
        vTaskList(pstr2);

#if ( configGENERATE_RUN_TIME_STATS == 1 )
        KMRAW("\r\n%s \r\n", pstr1);
#endif
        KMRAW("\r\n%s \r\n", pstr2);

        vTaskDelay(1000);
    }
}

int freertos_task_test(int argc, char **argv)
{
    xTaskCreate(task_1, "task_1", 512, NULL, 3, NULL);
    xTaskCreate(task_2, "task_2", 512, NULL, 3, NULL);

    return 0;
}
