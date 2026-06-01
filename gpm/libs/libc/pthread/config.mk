ifeq (${MK_USE_KERNEL_POSIX_PTHREAD},y)
PROJ_CDEFS += CONFIG_MODULE_KPOSIX_PTHREAD
CSOURCES += ${GPMPATH}/libs/libc/pthread/prv_pthread.c
CSOURCES += ${GPMPATH}/libs/libc/pthread/pthread.c
CSOURCES += ${GPMPATH}/libs/libc/pthread/pthread_attr.c
CSOURCES += ${GPMPATH}/libs/libc/pthread/pthread_barrier.c
CSOURCES += ${GPMPATH}/libs/libc/pthread/pthread_cond.c
CSOURCES += ${GPMPATH}/libs/libc/pthread/pthread_mutex.c
CSOURCES += ${GPMPATH}/libs/libc/pthread/pthread_rwlock.c
CSOURCES += ${GPMPATH}/libs/libc/pthread/pthread_spin.c

MK_USE_KERNEL_POSIX_TIME:=y
endif
