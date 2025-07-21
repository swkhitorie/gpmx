
# default simple device driver interface
CSOURCES   +=  libs/device/dnode.c
CSOURCES   +=  libs/device/serial.c
CSOURCES   +=  libs/device/i2c_master.c
CSOURCES   +=  libs/device/spi.c
CPPSOURCES +=  libs/device/spi.cpp
CPPSOURCES +=  libs/device/ri2c.cpp

ifeq (${MK_COMPILER},gcc)
CSOURCES   +=  libs/noneuse_syscall.c
endif

ifeq (${MK_USE_LIB_CPP},y)
PROJ_CINCDIRS   +=  libs/cpp
CPPSOURCES +=  libs/cpp/libxx_delete.cpp
CPPSOURCES +=  libs/cpp/libxx_delete_sized.cpp
CPPSOURCES +=  libs/cpp/libxx_deletea.cpp
CPPSOURCES +=  libs/cpp/libxx_deletea_sized.cpp
CPPSOURCES +=  libs/cpp/libxx_new.cpp
CPPSOURCES +=  libs/cpp/libxx_newa.cpp
endif # end with MK_USE_LIB_CPP

ifeq (${MK_USE_FS_FATFS},y)
PROJ_CINCDIRS += libs/fs/fat/
CSOURCES += libs/fs/fat/ff.c
CSOURCES += libs/fs/fat/diskio.c
CSOURCES += libs/fs/fat/ff_drv.c
CSOURCES += libs/fs/fat/ffsystem.c
CSOURCES += libs/fs/fat/ffunicode.c
endif # end with MK_USE_FS_FATFS - y

ifeq (${MK_USE_FS_FATFS},l)
PROJ_CINCDIRS += libs/fs/fat/legacy
CSOURCES += libs/fs/fat/legacy/ff.c
CSOURCES += libs/fs/fat/legacy/diskio.c
CSOURCES += libs/fs/fat/legacy/ff_drv.c
CSOURCES += libs/fs/fat/legacy/option/syscall.c
CSOURCES += libs/fs/fat/legacy/option/unicode.c
endif # end with MK_USE_FS_FATFS - l

ifeq (${MK_USE_FREERTOS},y)

ifeq (${MK_USE_FR_POSIX},y)
CSOURCES += libs/queue/dq_addafter.c
CSOURCES += libs/queue/dq_addbefore.c
CSOURCES += libs/queue/dq_addfirst.c
CSOURCES += libs/queue/dq_addlast.c
CSOURCES += libs/queue/dq_cat.c
CSOURCES += libs/queue/dq_count.c
CSOURCES += libs/queue/dq_rem.c
CSOURCES += libs/queue/dq_remfirst.c
CSOURCES += libs/queue/dq_remlast.c
CSOURCES += libs/queue/sq_addafter.c
CSOURCES += libs/queue/sq_addfirst.c
CSOURCES += libs/queue/sq_addlast.c
CSOURCES += libs/queue/sq_cat.c
CSOURCES += libs/queue/sq_count.c
CSOURCES += libs/queue/sq_rem.c
CSOURCES += libs/queue/sq_remafter.c
CSOURCES += libs/queue/sq_remfirst.c
CSOURCES += libs/queue/sq_remlast.c

CSOURCES += libs/mqueue/mq_close.c
CSOURCES += libs/mqueue/mq_getattr.c
CSOURCES += libs/mqueue/mq_open.c
CSOURCES += libs/mqueue/mq_receive.c
CSOURCES += libs/mqueue/mq_send.c
CSOURCES += libs/mqueue/mq_timedreceive.c
CSOURCES += libs/mqueue/mq_timedsend.c
CSOURCES += libs/mqueue/mq_unlink.c
CSOURCES += libs/mqueue/prv_mqueue.c

CSOURCES += libs/pthread/pthread.c
CSOURCES += libs/pthread/pthread_attr.c
CSOURCES += libs/pthread/pthread_barrier.c
CSOURCES += libs/pthread/pthread_cond.c
CSOURCES += libs/pthread/pthread_mutex.c
CSOURCES += libs/pthread/sched.c

CSOURCES += libs/semaphore/sem_destroy.c
CSOURCES += libs/semaphore/sem_getvalue.c
CSOURCES += libs/semaphore/sem_init.c
CSOURCES += libs/semaphore/sem_post.c
CSOURCES += libs/semaphore/sem_timedwait.c
CSOURCES += libs/semaphore/sem_trywait.c
CSOURCES += libs/semaphore/sem_wait.c

CSOURCES += libs/time/clock.c
CSOURCES += libs/time/clock_getres.c
CSOURCES += libs/time/clock_gettime.c
CSOURCES += libs/time/clock_nanosleep.c
CSOURCES += libs/time/clock_settime.c
CSOURCES += libs/time/lib_nanosleep.c

CSOURCES += libs/timer/prv_timer.c
CSOURCES += libs/timer/timer_create.c
CSOURCES += libs/timer/timer_delete.c
CSOURCES += libs/timer/timer_getoverrun.c
CSOURCES += libs/timer/timer_gettime.c
CSOURCES += libs/timer/timer_settime.c

CSOURCES += libs/unistd/lib_sleep.c
CSOURCES += libs/unistd/lib_usleep.c

CSOURCES += libs/utils.c
endif # end with MK_USE_FR_POSIX

endif # end with MK_USE_FREERTOS
