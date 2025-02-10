#include "cli_console.h"
#include <FreeRTOS.h>

static int cmd_free(int argc, char **argv)
{
    fprintf(stdout, "[freertos] total memory size: %d, free size: %d\r\n", 
        configADJUSTED_HEAP_SIZE, 
        xPortGetFreeHeapSize());
    return 0;
}

static char task_list_str[512];
static int cmd_ps(int argc, char **argv)
{
    const char *const header = "Task       State     Priority  Stack	#\r\n************************************************\r\n";
    strcpy(task_list_str, header);
    vTaskList(task_list_str + strlen(header));
    fprintf(stdout, "%s\r\n", task_list_str);
    return 0;
}

static int cmd_time(int argc, char **argv)
{
    float now = 
    fprintf(stdout, "%\r\n", task_list_str);
    return 0;
}

static const struct cmdmap_s g_cmdmap[] =
{
    { "free",     cmd_free,    0, 0, "free: display remain memory size" },
    { "ps",       cmd_ps,      0, 0, "ps: display task list" },
    { "time",     cmd_time,    0, 0, "time: display running timestamp" },
};

int cli_console_app_process()
{

}
