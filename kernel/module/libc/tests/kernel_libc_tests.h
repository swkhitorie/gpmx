#ifndef KERNEL_LIBC_TESTS_H_
#define KERNEL_LIBC_TESTS_H_

#include <board_config.h>

#ifndef TEST_PRINTF
#define TEST_PRINTF    BOARD_PRINTF
#endif

#ifdef __cplusplus
extern "C" {
#endif

int klibc_clock_test(int argc, char **argv);
int klibc_mq_test(int argc, char **argv);
int klibc_pthread_mutex_test(int argc, char **argv);
int klibc_pthread_test(int argc, char **argv);
int klibc_sem_test(int argc, char **argv);
int klibc_timer_test(int argc, char **argv);

#ifdef __cplusplus
}
#endif


#endif
