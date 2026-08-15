
ifeq (${CONFIG_LIBC_TIME},y)
CSOURCES += ${GPMPATH}/tests/libc/klibc_clock_test.c
endif

ifeq (${CONFIG_LIBC_MQUEUE},y)
CSOURCES += ${GPMPATH}/tests/libc/klibc_mq_test.c
endif

ifeq (${CONFIG_LIBC_PTHREAD},y)
CSOURCES += ${GPMPATH}/tests/libc/klibc_pthread_barrier_test.c
CSOURCES += ${GPMPATH}/tests/libc/klibc_pthread_cond_test.c
CSOURCES += ${GPMPATH}/tests/libc/klibc_pthread_mutex_test.c
CSOURCES += ${GPMPATH}/tests/libc/klibc_pthread_rwlock_test.c
CSOURCES += ${GPMPATH}/tests/libc/klibc_pthread_spinlock_test.c
CSOURCES += ${GPMPATH}/tests/libc/klibc_pthread_test.c
endif

ifeq (${CONFIG_LIBC_SEMAPHORE},y)
CSOURCES += ${GPMPATH}/tests/libc/klibc_sem_test.c
CSOURCES += ${GPMPATH}/tests/libc/klibc_sem_reverse_test.c
endif

ifeq (${CONFIG_LIBC_TIMER},y)
CSOURCES += ${GPMPATH}/tests/libc/klibc_timer_test.c
endif
