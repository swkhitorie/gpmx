
ifeq (${MK_COMPILER},gcc)
CSOURCES   +=  libs/libc/noneuse_syscall.c
endif

ifeq (${MK_RTOS},frtos)

ifeq (${MK_USE_POSIX},y)
PROJ_CINCDIRS += libs/libc/_inc

CSOURCES += libs/libc/queue/dq_addafter.c
CSOURCES += libs/libc/queue/dq_addbefore.c
CSOURCES += libs/libc/queue/dq_addfirst.c
CSOURCES += libs/libc/queue/dq_addlast.c
CSOURCES += libs/libc/queue/dq_cat.c
CSOURCES += libs/libc/queue/dq_count.c
CSOURCES += libs/libc/queue/dq_rem.c
CSOURCES += libs/libc/queue/dq_remfirst.c
CSOURCES += libs/libc/queue/dq_remlast.c
CSOURCES += libs/libc/queue/sq_addafter.c
CSOURCES += libs/libc/queue/sq_addfirst.c
CSOURCES += libs/libc/queue/sq_addlast.c
CSOURCES += libs/libc/queue/sq_cat.c
CSOURCES += libs/libc/queue/sq_count.c
CSOURCES += libs/libc/queue/sq_rem.c
CSOURCES += libs/libc/queue/sq_remafter.c
CSOURCES += libs/libc/queue/sq_remfirst.c
CSOURCES += libs/libc/queue/sq_remlast.c

CSOURCES += libs/libc/mqueue/mq_close.c
CSOURCES += libs/libc/mqueue/mq_getattr.c
CSOURCES += libs/libc/mqueue/mq_open.c
CSOURCES += libs/libc/mqueue/mq_receive.c
CSOURCES += libs/libc/mqueue/mq_send.c
CSOURCES += libs/libc/mqueue/mq_timedreceive.c
CSOURCES += libs/libc/mqueue/mq_timedsend.c
CSOURCES += libs/libc/mqueue/mq_unlink.c
CSOURCES += libs/libc/mqueue/prv_mqueue.c

CSOURCES += libs/libc/pthread/pthread.c
CSOURCES += libs/libc/pthread/pthread_attr.c
CSOURCES += libs/libc/pthread/pthread_barrier.c
CSOURCES += libs/libc/pthread/pthread_cond.c
CSOURCES += libs/libc/pthread/pthread_mutex.c
CSOURCES += libs/libc/pthread/sched.c

CSOURCES += libs/libc/semaphore/sem_destroy.c
CSOURCES += libs/libc/semaphore/sem_getvalue.c
CSOURCES += libs/libc/semaphore/sem_init.c
CSOURCES += libs/libc/semaphore/sem_post.c
CSOURCES += libs/libc/semaphore/sem_timedwait.c
CSOURCES += libs/libc/semaphore/sem_trywait.c
CSOURCES += libs/libc/semaphore/sem_wait.c

CSOURCES += libs/libc/time/clock.c
CSOURCES += libs/libc/time/clock_getres.c
CSOURCES += libs/libc/time/clock_gettime.c
CSOURCES += libs/libc/time/clock_nanosleep.c
CSOURCES += libs/libc/time/clock_settime.c
CSOURCES += libs/libc/time/lib_nanosleep.c

CSOURCES += libs/libc/timer/prv_timer.c
CSOURCES += libs/libc/timer/timer_create.c
CSOURCES += libs/libc/timer/timer_delete.c
CSOURCES += libs/libc/timer/timer_getoverrun.c
CSOURCES += libs/libc/timer/timer_gettime.c
CSOURCES += libs/libc/timer/timer_settime.c

CSOURCES += libs/libc/unistd/lib_sleep.c
CSOURCES += libs/libc/unistd/lib_usleep.c

CSOURCES += libs/libc/utils.c
endif # end with MK_USE_POSIX

endif # end with MK_RTOS
