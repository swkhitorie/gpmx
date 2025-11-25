#include "app_cli.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

__attribute__((weak)) void fcli_writer(char ch)
{
    // need redirect fwrite() to block send
    fwrite(&ch, 1, 1, stdout);
}

__attribute__((weak)) int fcli_read(char *ch)
{
    // need redirect fread()
    return fread(ch, 1, 1, stdin);
}

#ifdef CONFIG_BOARD_FREERTOS_ENABLE
#include <FreeRTOS.h>
#include <task.h>
static int cmd_free(int argc, char **argv)
{
    printf("[freertos] heap memory size: %d, free size: %d\r\n", 
        configTOTAL_HEAP_SIZE, 
        xPortGetFreeHeapSize());
    return 0;
}

static int cmd_ps(int argc, char **argv)
{
    char task_list_str[512];
    const char *const header = "Task         State   Priority  Stack	#\r\n************************************************\r\n";
    strcpy(task_list_str, header);
    vTaskList(task_list_str + strlen(header));
    printf("%s\r\n", task_list_str);
    return 0;
}

static int cmd_time(int argc, char **argv)
{
    TickType_t tick = xTaskGetTickCount();
    float now = tick * 1.0f / (float)configTICK_RATE_HZ;
    printf("[freertos] running time: %.4f\r\n", now);
    return 0;
}

static int cmd_top(int argc, char **argv)
{
    char top_list_str[512];
    if (configGENERATE_RUN_TIME_STATS != 1) {
        printf("now config not support, please check macro configGENERATE_RUN_TIME_STATS\r\n");
    } else {
        vTaskGetRunTimeStats(top_list_str);
        const char *const header = "Task            RTime          Load#\r\n************************************\r\n";
        strcpy(top_list_str, header);
        vTaskGetRunTimeStats(top_list_str + strlen(header));
        printf("%s\r\n", top_list_str);
    }
    return 0;
}

static const struct cli_cmdmap_s base_cmdmap[] =
{
    { "free",     cmd_free,    1, 1, "free: display remain memory size" },
    { "ps",       cmd_ps,      1, 1, "ps: display task list" },
    { "time",     cmd_time,    1, 1, "time: display running timestamp" },
    { "top",      cmd_top,     1, 1, "top: show tasks cpu load" },
};
#endif

#ifdef CONFIG_BOARD_FREERTOS_ENABLE
void fcli_task(void *p)
{
    for (;;) {
        cli_task();
        //scan period: 10ms
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}
#endif


void fcli_init()
{
    int i;
    cli_init(fcli_read, fcli_writer, NULL);

#ifdef CONFIG_BOARD_FREERTOS_ENABLE
    for (i = 0; i < sizeof(base_cmdmap) / sizeof(base_cmdmap[0]) ;i++) {
        cli_register(
            base_cmdmap[i].cmd, 
            base_cmdmap[i].handler, 
            base_cmdmap[i].minargs, 
            base_cmdmap[i].maxargs, 
            base_cmdmap[i].usage);
    }
#endif

#ifdef CONFIG_FRCLI_EXCMD_NUM
for (i = 0; i < sizeof(ex_cmdmap) / sizeof(ex_cmdmap[0]) ;i++) {
    cli_register(
        ex_cmdmap[i].cmd, 
        ex_cmdmap[i].handler, 
        ex_cmdmap[i].minargs, 
        ex_cmdmap[i].maxargs, 
        ex_cmdmap[i].usage);
}
#endif
#ifdef CONFIG_BOARD_FREERTOS_ENABLE
    xTaskCreate(fcli_task, "cli", 1024, NULL, 1, NULL);
#endif

}

