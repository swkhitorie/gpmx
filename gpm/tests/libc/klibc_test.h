#ifndef KLIBC_TEST_H_
#define KLIBC_TEST_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CONFIG_LIBC_TIME)
int klibc_clock_test(int argc, char **argv);
#endif


#if defined(CONFIG_LIBC_MQUEUE)
int klibc_mq_test(int argc, char **argv);
#endif

#if defined(CONFIG_LIBC_SEMAPHORE)
int klibc_sem_test(int argc, char **argv);
int klibc_sem_reverse_test(int argc, char **argv);
#endif

#if defined(CONFIG_LIBC_TIMER)
int klibc_timer_test(int argc, char **argv);
#endif

#if defined(CONFIG_LIBC_PTHREAD)
int klibc_pthread_test(int argc, char **argv);
int klibc_pthread_barrier_test(int argc, char **argv);
int klibc_pthread_cond_test(int argc, char **argv);
int klibc_pthread_mutex_test(int argc, char **argv);
int klibc_pthread_rwlock_test(int argc, char **argv);
int klibc_pthread_spinlock_test(int argc, char **argv);
#endif

#ifdef __cplusplus
}
#endif

#endif
