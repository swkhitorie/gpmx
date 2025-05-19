#include "board_config.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

#include "app_posix_test.h"

int main(void)
{
    board_init();
    printf("\r\n[app] bsp init completed \r\n");

    hrt_init();

    app_posix_freertos_debug_init();
    sched_start();
    for (;;);
}
