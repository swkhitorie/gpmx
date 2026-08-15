#include "px_platform.h"
#include "main.h"

static void pxlog_output(const char *buf)
{
    fputs(buf, stdout);
}

void px_platform_init()
{
    px4_log_streamouthook_set(pxlog_output);

    hrt_init();
    workqueue_manager_start();
    work_queues_init();
    uorb_manager_initialize();

    param_init();

    param_set_default_file(APP_PROJ_PATH"/test_params");
    param_set_backup_file(APP_PROJ_PATH"./backup_test_params");
    param_save_default();

    // uorb publisher rule setting
    char *file_content = NULL;
    long file_size = 0;
    FILE *fp1;
    fp1 = fopen(APP_PROJ_PATH"/orb_publisher.rules", "r");
    if (fp1) {
        fseek(fp1, 0, SEEK_END);
        file_size = ftell(fp1);
        rewind(fp1);

        file_content = (char *)malloc(file_size + 1);
        size_t read_len = fread(file_content, 1, file_size, fp1);
        if (read_len != (size_t)file_size) {
            printf("file read error \r\n");
            fclose(fp1);
            free(file_content);
            return;
        }
        file_content[file_size] = '\0';
    } else {
        printf("open %s failed\r\n", APP_PROJ_PATH"/orb_publisher.rules");
    }

    publisher_rule_config(file_content, file_size);
    publisher_rule_printl();
}
