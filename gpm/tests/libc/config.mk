
ifeq (${MK_TEST_ENABLE},y)

ifeq (${MK_USE_KERNEL_POSIX_TIME},y)
CSOURCES += ${GPMPATH}/tests/libc/klibc_clock_test.c
endif

ifeq (${MK_USE_KERNEL_POSIX_MQUEUE},y)
CSOURCES += ${GPMPATH}/tests/libc/klibc_mq_test.c
endif

ifeq (${MK_USE_KERNEL_POSIX_PTHREAD},y)
CSOURCES += ${GPMPATH}/tests/libc/klibc_pthread_barrier_test.c
CSOURCES += ${GPMPATH}/tests/libc/klibc_pthread_cond_test.c
CSOURCES += ${GPMPATH}/tests/libc/klibc_pthread_mutex_test.c
CSOURCES += ${GPMPATH}/tests/libc/klibc_pthread_rwlock_test.c
CSOURCES += ${GPMPATH}/tests/libc/klibc_pthread_spinlock_test.c
CSOURCES += ${GPMPATH}/tests/libc/klibc_pthread_test.c
endif

ifeq (${MK_USE_KERNEL_POSIX_SEMAPHORE},y)
CSOURCES += ${GPMPATH}/tests/libc/klibc_sem_test.c
endif

ifeq (${MK_USE_KERNEL_POSIX_TIMER},y)
CSOURCES += ${GPMPATH}/tests/libc/klibc_timer_test.c
endif

endif
