#include "board_config.h"
#include <stdio.h>
#include <drivers/drv_hrt.h>

#include "app_fatfs_test.h"
#include "app_cli.h"
#include <FreeRTOS.h>
#include <task.h>

static int cmd_ls(int argc, char **argv)
{
    if (argc != 2) {
        printf("ls usage: ls <path[fatfs format]> \r\n");
    } else {
        ff_ls(argv[1]);
    }
    return 0;
}

static int cmd_cat(int argc, char **argv)
{
    if (argc != 2) {
        printf("cat usage: cat <path[fatfs format]> \r\n");
    } else {
        ff_cat(argv[1]);
    }
    return 0;
}

const struct cli_cmdmap_s ex_cmdmap[CONFIG_FRCLI_EXCMD_NUM] = 
{
    { "ls",     cmd_ls,    1, 2, "ls  <path[fatfs format]>" },
    { "cat",    cmd_cat,   1, 2, "cat <path[fatfs format]>" },
};

void fr_heart(void *p)
{
    for (;;) {
        board_blue_led_toggle();
        vTaskDelay(250/portTICK_PERIOD_MS);
    }
}

int main(void)
{
    board_init();

    hrt_init();
    fcli_init();

    xTaskCreate(fr_heart, "heart", 1024, NULL, 3, NULL);
    vTaskStartScheduler();
    for (;;);
}
