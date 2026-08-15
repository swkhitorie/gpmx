#ifndef _FREERTOS_TEST_INCLUDE_H_
#define _FREERTOS_TEST_INCLUDE_H_


#ifdef __cplusplus
extern "C" {
#endif

int freertos_eventgroup_test(int argc, char **argv);
int freertos_messagebuffer_test(int argc, char **argv);
int freertos_mutex_test(int argc, char **argv);
int freertos_priority_reverse_test(int argc, char **argv);
int freertos_queue_test(int argc, char **argv);
int freertos_sem_test(int argc, char **argv);
int freertos_status_test(int argc, char **argv);
int freertos_streambuffer_test(int argc, char **argv);
int freertos_task_test(int argc, char **argv);
int freertos_timer_test(int argc, char **argv);

#ifdef __cplusplus
}
#endif


#endif
