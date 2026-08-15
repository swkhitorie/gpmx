ifeq (${CONFIG_LIBC_PTHREAD},y)

ifeq (${CONFIG_LIBC_TIME},n)
$(error CONFIG_LIBC_PTHREAD depend on CONFIG_LIBC_TIME)
endif

CSOURCES += ${GPMPATH}/libs/libc/pthread/prv_pthread.c
CSOURCES += ${GPMPATH}/libs/libc/pthread/pthread.c
CSOURCES += ${GPMPATH}/libs/libc/pthread/pthread_attr.c
CSOURCES += ${GPMPATH}/libs/libc/pthread/pthread_barrier.c
CSOURCES += ${GPMPATH}/libs/libc/pthread/pthread_cond.c
CSOURCES += ${GPMPATH}/libs/libc/pthread/pthread_mutex.c
CSOURCES += ${GPMPATH}/libs/libc/pthread/pthread_rwlock.c
CSOURCES += ${GPMPATH}/libs/libc/pthread/pthread_spin.c
CSOURCES += ${GPMPATH}/libs/libc/pthread/pthread_tls.c
endif
