
# default simple device driver interface
FR_CSOURCES   +=  libs/device/dnode.c
FR_CSOURCES   +=  libs/device/serial.c
FR_CSOURCES   +=  libs/device/i2c_master.c
FR_CSOURCES   +=  libs/device/spi.c

ifeq (${CONFIG_MK_COMPILER},gcc)
FR_CSOURCES   +=  libs/noneuse_syscall.c
endif

ifeq (${CONFIG_MK_USE_LIB_CPP},y)
FR_CINCDIRS   +=  libs/cpp
FR_CPPSOURCES +=  libs/cpp/libxx_delete.cpp
FR_CPPSOURCES +=  libs/cpp/libxx_delete_sized.cpp
FR_CPPSOURCES +=  libs/cpp/libxx_deletea.cpp
FR_CPPSOURCES +=  libs/cpp/libxx_deletea_sized.cpp
FR_CPPSOURCES +=  libs/cpp/libxx_new.cpp
FR_CPPSOURCES +=  libs/cpp/libxx_newa.cpp
endif # end with CONFIG_MK_USE_LIB_CPP

ifeq (${CONFIG_MK_USE_FS_FATFS},y)
FR_CINCDIRS += libs/fs/fat/
FR_CSOURCES += libs/fs/fat/ff.c
FR_CSOURCES += libs/fs/fat/diskio.c
FR_CSOURCES += libs/fs/fat/ff_drv.c
FR_CSOURCES += libs/fs/fat/ffsystem.c
FR_CSOURCES += libs/fs/fat/ffunicode.c
endif # end with CONFIG_MK_USE_FS_FATFS - y

ifeq (${CONFIG_MK_USE_FS_FATFS},l)
FR_CINCDIRS += libs/fs/fat/legacy
FR_CSOURCES += libs/fs/fat/legacy/ff.c
FR_CSOURCES += libs/fs/fat/legacy/diskio.c
FR_CSOURCES += libs/fs/fat/legacy/ff_drv.c
FR_CSOURCES += libs/fs/fat/legacy/option/syscall.c
FR_CSOURCES += libs/fs/fat/legacy/option/unicode.c
endif # end with CONFIG_MK_USE_FS_FATFS - l

ifeq (${CONFIG_MK_USE_FREERTOS},y)

ifeq (${CONFIG_MK_USE_FR_POSIX},y)
FR_CSOURCES += libs/queue/dq_addafter.c
FR_CSOURCES += libs/queue/dq_addbefore.c
FR_CSOURCES += libs/queue/dq_addfirst.c
FR_CSOURCES += libs/queue/dq_addlast.c
FR_CSOURCES += libs/queue/dq_cat.c
FR_CSOURCES += libs/queue/dq_count.c
FR_CSOURCES += libs/queue/dq_rem.c
FR_CSOURCES += libs/queue/dq_remfirst.c
FR_CSOURCES += libs/queue/dq_remlast.c
FR_CSOURCES += libs/queue/sq_addafter.c
FR_CSOURCES += libs/queue/sq_addfirst.c
FR_CSOURCES += libs/queue/sq_addlast.c
FR_CSOURCES += libs/queue/sq_cat.c
FR_CSOURCES += libs/queue/sq_count.c
FR_CSOURCES += libs/queue/sq_rem.c
FR_CSOURCES += libs/queue/sq_remafter.c
FR_CSOURCES += libs/queue/sq_remfirst.c
FR_CSOURCES += libs/queue/sq_remlast.c

FR_CSOURCES += libs/mqueue/mq_close.c
FR_CSOURCES += libs/mqueue/mq_getattr.c
FR_CSOURCES += libs/mqueue/mq_open.c
FR_CSOURCES += libs/mqueue/mq_receive.c
FR_CSOURCES += libs/mqueue/mq_send.c
FR_CSOURCES += libs/mqueue/mq_timedreceive.c
FR_CSOURCES += libs/mqueue/mq_timedsend.c
FR_CSOURCES += libs/mqueue/mq_unlink.c
FR_CSOURCES += libs/mqueue/prv_mqueue.c

FR_CSOURCES += libs/pthread/pthread.c
FR_CSOURCES += libs/pthread/pthread_attr.c
FR_CSOURCES += libs/pthread/pthread_barrier.c
FR_CSOURCES += libs/pthread/pthread_cond.c
FR_CSOURCES += libs/pthread/pthread_mutex.c
FR_CSOURCES += libs/pthread/sched.c

FR_CSOURCES += libs/semaphore/sem_destroy.c
FR_CSOURCES += libs/semaphore/sem_getvalue.c
FR_CSOURCES += libs/semaphore/sem_init.c
FR_CSOURCES += libs/semaphore/sem_post.c
FR_CSOURCES += libs/semaphore/sem_timedwait.c
FR_CSOURCES += libs/semaphore/sem_trywait.c
FR_CSOURCES += libs/semaphore/sem_wait.c

FR_CSOURCES += libs/time/clock.c
FR_CSOURCES += libs/time/clock_getres.c
FR_CSOURCES += libs/time/clock_gettime.c
FR_CSOURCES += libs/time/clock_nanosleep.c
FR_CSOURCES += libs/time/clock_settime.c
FR_CSOURCES += libs/time/lib_nanosleep.c

FR_CSOURCES += libs/timer/prv_timer.c
FR_CSOURCES += libs/timer/timer_create.c
FR_CSOURCES += libs/timer/timer_delete.c
FR_CSOURCES += libs/timer/timer_getoverrun.c
FR_CSOURCES += libs/timer/timer_gettime.c
FR_CSOURCES += libs/timer/timer_settime.c

FR_CSOURCES += libs/unistd/lib_sleep.c
FR_CSOURCES += libs/unistd/lib_usleep.c

FR_CSOURCES += libs/utils.c
endif # end with CONFIG_MK_USE_FR_POSIX

endif # end with CONFIG_MK_USE_FREERTOS

ifeq (${CONFIG_MK_USE_HRT},y)
FR_CINCDIRS   +=  libs/hrt/
FR_CSOURCES   +=  libs/hrt/hrt.c
ifeq (${CONFIG_MK_USE_FR_POSIX},n)
FR_CINCDIRS += libs/hrt/queue
FR_CSOURCES += libs/queue/dq_addafter.c
FR_CSOURCES += libs/queue/dq_addbefore.c
FR_CSOURCES += libs/queue/dq_addfirst.c
FR_CSOURCES += libs/queue/dq_addlast.c
FR_CSOURCES += libs/queue/dq_cat.c
FR_CSOURCES += libs/queue/dq_count.c
FR_CSOURCES += libs/queue/dq_rem.c
FR_CSOURCES += libs/queue/dq_remfirst.c
FR_CSOURCES += libs/queue/dq_remlast.c
FR_CSOURCES += libs/queue/sq_addafter.c
FR_CSOURCES += libs/queue/sq_addfirst.c
FR_CSOURCES += libs/queue/sq_addlast.c
FR_CSOURCES += libs/queue/sq_cat.c
FR_CSOURCES += libs/queue/sq_count.c
FR_CSOURCES += libs/queue/sq_rem.c
FR_CSOURCES += libs/queue/sq_remafter.c
FR_CSOURCES += libs/queue/sq_remfirst.c
FR_CSOURCES += libs/queue/sq_remlast.c
endif # CONFIG_MK_USE_FR_POSIX
endif # CONFIG_MK_USE_HRT

ifeq (${CONFIG_MK_USE_UORB},y)
FR_CINCDIRS   +=  libs/uorb/include
FR_CINCDIRS   +=  libs/uorb/src
FR_CPPSOURCES +=  libs/uorb/src/device_master.cpp
FR_CPPSOURCES +=  libs/uorb/src/device_node.cpp
FR_CPPSOURCES +=  libs/uorb/src/uorb.cpp
include ${SDK_ROOTDIR}/libs/uorb_msgs/make.mk
endif

ifeq (${CONFIG_MK_USE_PX4_LOG},y)
FR_CPPSOURCES +=  libs/px4_log.cpp
endif

ifeq (${CONFIG_MK_USE_PX4_MODULE},y)
FR_CPPSOURCES +=  libs/module.cpp
endif

ifeq (${CONFIG_MK_USE_PX4_TASKS},y)
FR_CPPSOURCES +=  libs/px4_tasks.cpp
endif

ifeq (${CONFIG_MK_USE_PERF_CNTER},y)
FR_CPPSOURCES +=  libs/perf_counter.cpp
endif

