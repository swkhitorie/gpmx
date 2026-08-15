#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include "main.h"
#include "px_platform.h"

#include "cJSON.h"
#include "pserial.h"
#include <errno.h>

int init_config(char *devpath)
{
    FILE *file = fopen(APP_PROJ_PATH"/config.json", "r");
    if (!file) {
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *data = (char *)malloc(length + 1);
    fread(data, 1, length, file);
    fclose(file);
    data[length] = '\0';

    cJSON *json = cJSON_Parse(data);
    if (json == NULL) {
        free(data);
        return -2;
    }

    cJSON *name = cJSON_GetObjectItem(json, "port");
    strcpy(devpath, name->valuestring);

    cJSON_Delete(json);
    free(data);
    return 0;
}
#include "nshlib/nsh.h"

#include "modules/replay/Replay.hpp"
extern "C" {extern int replay_main(int argc, char *argv[]);}

int main(int argc, char **argv)
{
    setbuf(stdout, NULL);   // make stream stdout unbuffered
    pthread_setname_np(pthread_self(), "test1");

    char port_path[50] = {'\0'};
    if (init_config(port_path)) {
        printf("[init] error parsing json\n");
    }

    printf("[init] open port %s\n", port_path);
    pserial_recving_start((const char *)port_path);

    px_platform_init();

    nsh_consolemain(0, NULL);

    // char *argv_1[] = {"replay", "start", NULL};
    // int argc_1 = 3;
    // replay_main(argc_1, argv_1);

    while (1) {

        // uorb_device_master_print_statistics(uorb_manager_get_device_master(uorb_manager_instance()));

        // printf("heartbeat\r\n");
        usleep(1000*1000);
    }

    return 0;
}
