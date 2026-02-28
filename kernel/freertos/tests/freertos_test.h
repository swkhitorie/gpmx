#ifndef FREERTOS_TEST_H_
#define FREERTOS_TEST_H_

#include <board_config.h>

#ifndef TEST_PRINTF
#define TEST_PRINTF    BOARD_PRINTF
#endif

#ifdef __cplusplus
extern "C" {
#endif

int freertos_mutex_test(int argc, char **argv);
int freertos_priority_reverse_test(int argc, char **argv);
int freertos_queue_test(int argc, char **argv);
int freertos_sem_test(int argc, char **argv);
int freertos_task_test(int argc, char **argv);
int freertos_timer_test(int argc, char **argv);
int freertos_status_test(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif
