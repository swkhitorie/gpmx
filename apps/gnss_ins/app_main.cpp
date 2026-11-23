#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include <gtest/gtest.h>

#include "nshlib/nsh.h"
#include "drivers/drv_hrt.h"
#include "workqueue/workqueue_manager.h"
#include "wqueue/wqueue.h"
#include "uorb/uorb_common.h"
#include "uorb/uorb_manager.h"

#include "platform/common/log.h"
#include "lib/parameters/param.h"
#include "cJSON.h"

int modules_config()
{
	char *json_path = (char *)CONFIG_SRC_ROOTDIR"/config/cfg.json";
	char *rule_path = (char *)CONFIG_SRC_ROOTDIR"/config/orb_publisher.rules";

	/* replay path env config */
    FILE *file = fopen(json_path, "r");
    if (!file) {
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *data = (char *)malloc(length + 1);
    size_t rsz = fread(data, 1, length, file);
    if (rsz != length) {
        return -3;
    }
    fclose(file);
    data[length] = '\0';

    cJSON *json = cJSON_Parse(data);
    if (json == NULL) {
        free(data);
        return -2;
    }

    cJSON *name = cJSON_GetObjectItem(json, "replay_path");
	setenv("replay", name->valuestring, 1);
    // setenv("replay_mode", "ekf2", 1);
    // setenv("PX4_SIM_SPEED_FACTOR", "1", 1);

    cJSON_Delete(json);
    free(data);

	/* uorb publisher rule config */
    char *file_content = NULL;
    long file_size = 0;
    FILE *fp1;
    fp1 = fopen(rule_path, "r");
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
            return -2;
        }
        file_content[file_size] = '\0';
    } else {
        printf("open %s failed\r\n", rule_path);
    }

    publisher_rule_config(file_content, strlen(file_content));
    publisher_rule_printl();

    return 0;
}

int main(int argc, char **argv)
{
    setbuf(stdout, NULL);

#if defined(CONFIG_TEST)
    testing::InitGoogleTest(&argc, argv);
    for (int i = 1; i < argc; i++) {
        if (argv[i] && !strcmp(argv[i], "--run-tests")) {
            return RUN_ALL_TESTS();
        }
    }
#endif

    hrt_init();
    workqueue_manager_start();
    work_queues_init();

    uorb_manager_initialize();

    param_init();
    param_set_default_file(CONFIG_APP_ROOTDIR"params_test");
    param_set_backup_file(CONFIG_APP_ROOTDIR"params_test_bak");
    param_save_default();

    if (!modules_config()) {
        nsh_consolemain(0, NULL);
    }

    printf("[main] end\r\n");
    while (1) {}
    return 0;
}
