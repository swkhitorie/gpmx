ifeq (${MK_USE_KERNEL_POSIX_SEMAPHORE},y)
PROJ_CDEFS += CONFIG_MODULE_KPOSIX_SEMAPHORE
CSOURCES += ${GPMPATH}/libs/libc/semaphore/prv_sem.c
CSOURCES += ${GPMPATH}/libs/libc/semaphore/sem_close.c
CSOURCES += ${GPMPATH}/libs/libc/semaphore/sem_destroy.c
CSOURCES += ${GPMPATH}/libs/libc/semaphore/sem_getprotocol.c
CSOURCES += ${GPMPATH}/libs/libc/semaphore/sem_getvalue.c
CSOURCES += ${GPMPATH}/libs/libc/semaphore/sem_init.c
CSOURCES += ${GPMPATH}/libs/libc/semaphore/sem_open.c
CSOURCES += ${GPMPATH}/libs/libc/semaphore/sem_post.c
CSOURCES += ${GPMPATH}/libs/libc/semaphore/sem_setprotocol.c
CSOURCES += ${GPMPATH}/libs/libc/semaphore/sem_timedwait.c
CSOURCES += ${GPMPATH}/libs/libc/semaphore/sem_trywait.c
CSOURCES += ${GPMPATH}/libs/libc/semaphore/sem_unlink.c
CSOURCES += ${GPMPATH}/libs/libc/semaphore/sem_wait.c

MK_USE_KERNEL_POSIX_TIME:=y
endif
