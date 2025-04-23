#include "board_config.h"
#include <stdio.h>

#include "app_fatfs_debug.h"


void fr_heart(void *p)
{
    static char fr_debug_str[512];
    for (;;) {
        vTaskList(&fr_debug_str[0]);
        fprintf(stdout, "%s \r\n", fr_debug_str);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

int main(void)
{
    board_init();
    printf("\r\n[app] bsp init completed \r\n");

    hrt_init();

    ff_ls("0:/");
    ff_display("0:/my tech.txt");

    xTaskCreate(fr_heart, "ht_debug1", 1024, NULL, 3, NULL);
    vTaskStartScheduler();
    for (;;);
}
