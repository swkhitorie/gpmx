
FR_CSOURCES   +=  libs/dev/dnode.c
FR_CSOURCES   +=  libs/dev/serial.c
FR_CSOURCES   +=  libs/dev/i2c_master.c
FR_CSOURCES   +=  libs/dev/spi.c

ifeq (${CONFIG_FR_LIB_CXX},y)
FR_CINCDIRS   +=  libs/cxx
FR_CPPSOURCES +=  libs/cxx/libxx_delete.cxx
FR_CPPSOURCES +=  libs/cxx/libxx_delete_sized.cxx
FR_CPPSOURCES +=  libs/cxx/libxx_deletea.cxx
FR_CPPSOURCES +=  libs/cxx/libxx_deletea_sized.cxx
FR_CPPSOURCES +=  libs/cxx/libxx_new.cxx
FR_CPPSOURCES +=  libs/cxx/libxx_newa.cxx
endif

ifeq (${CONFIG_FR_LIB_PX4_SUPPORT},y)
FR_CSOURCES   +=  libs/px4/hrt/hrt.c
FR_CPPSOURCES +=  libs/px4/ptasks.cpp
else
ifeq (${CONFIG_USE_DRV_HRT_INTERNAL},y)
FR_CSOURCES   +=  libs/px4/hrt/hrt.c
FR_CINCDIRS   +=  libs/px4
endif
endif

ifeq (${CONFIG_FR_FAT_FATFS},y)
FR_CINCDIRS += libs/fs/fat/
FR_CSOURCES += libs/fs/fat/ff.c
FR_CSOURCES += libs/fs/fat/diskio.c
FR_CSOURCES += libs/fs/fat/ff_drv.c
FR_CSOURCES += libs/fs/fat/ffsystem.c
FR_CSOURCES += libs/fs/fat/ffunicode.c
endif

ifeq (${CONFIG_FR_FAT_FATFS},l)
FR_CINCDIRS += libs/fs/fat/legacy
FR_CSOURCES += libs/fs/fat/legacy/ff.c
FR_CSOURCES += libs/fs/fat/legacy/diskio.c
FR_CSOURCES += libs/fs/fat/legacy/ff_drv.c
FR_CSOURCES += libs/fs/fat/legacy/option/syscall.c
FR_CSOURCES += libs/fs/fat/legacy/option/unicode.c
endif


ifeq (${CONFIG_FR_LIB_POSIX},y)
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
endif








