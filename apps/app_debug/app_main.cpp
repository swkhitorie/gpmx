#include "./app_main.h"
#include <stdio.h>

#include "app_posix_debug.h"
//#include "app_fatfs_debug.h"
//#include "app_cli.h"

// void debug_led_toggle()
// {
// #if   BOARD_SELECT == BOARD_FMUV2
//     board_led_toggle();
// #elif BOARD_SELECT == BOARD_FANKEH7
//     board_blue_led_toggle();
// #elif BOARD_SELECT == BOARD_FMUV6C
//     board_blue_led_toggle();
//     board_red_led_toggle();
// #endif
// }

// void fr_heart(void *p)
// {
//     static portTickType xLastWakeTime;  
//     const portTickType xFrequency = pdMS_TO_TICKS(500);
//     xLastWakeTime = xTaskGetTickCount();
//     for (;;) {
//         float a1 = HAL_GetTick()/1e3f;
//         float a2 = hrt_absolute_time()/1e6f;
//         debug_led_toggle();
//         // fprintf(stdout, "[heart] %s, kernel %.6f %.6f %.6f\r\n", 
//         //     pcTaskGetName(xTaskGetCurrentTaskHandle()), 
//         //     a1, a2, a2-a1);
//         vTaskDelayUntil(&xLastWakeTime, xFrequency);
//     }
// }

// void fr_heart2(void *p)
// {
//     static char fr_debug_str[512];
//     for (;;) {
//         vTaskList(&fr_debug_str[0]);
//         fprintf(stdout, "%s \r\n", fr_debug_str);
//         vTaskDelay(1000/portTICK_PERIOD_MS);
//     }
// }

// void debug()
// {
//     uint32_t tickm = HAL_GetTick();
//     if (HAL_GetTick() - tickm >= 100) {
//         tickm = HAL_GetTick();
//         fprintf(stdout, "[heart] stdio %s, kernel %d\r\n", 
//             pcTaskGetName(xTaskGetCurrentTaskHandle()), HAL_GetTick());
//         debug_led_toggle();
//     }
// }

int main(void)
{
    board_init();
    printf("\r\n[app] bsp init completed \r\n");

    //fcli_init();
    hrt_init();

    // ff_ls("0:/");
    // ff_display("0:/my tech.txt");

    // xTaskCreate(fr_heart, "ht_debug1", 1024, NULL, 3, NULL);
    // xTaskCreate(fr_heart2, "ht_debug2", 2048, NULL, 1, NULL);
    // vTaskStartScheduler();

    app_posix_freertos_debug_init();
    sched_start();
    for (;;);
}
