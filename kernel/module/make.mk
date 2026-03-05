
# general
PROJ_CINCDIRS += kernel/module/libc/include
PROJ_CINCDIRS += kernel/module/libc/tests
CSOURCES += kernel/module/libc/utils/utils.c

ifeq (${MK_USE_MODULE_KPRINTF},y)
PROJ_CDEFS += CONFIG_MODULE_KPRINTF
PROJ_CINCDIRS += kernel/module/kprintf
CSOURCES += kernel/module/kprintf/kprintf.c
endif

ifeq (${MK_USE_KERNEL_CPP},y)
PROJ_CDEFS += CONFIG_MODULE_KCPP
PROJ_CINCDIRS += kernel/module/libc/cxx
CPPSOURCES += kernel/module/libc/cxx/libxx_delete.cpp
CPPSOURCES += kernel/module/libc/cxx/libxx_delete_sized.cpp
CPPSOURCES += kernel/module/libc/cxx/libxx_deletea.cpp
CPPSOURCES += kernel/module/libc/cxx/libxx_deletea_sized.cpp
CPPSOURCES += kernel/module/libc/cxx/libxx_new.cpp
CPPSOURCES += kernel/module/libc/cxx/libxx_newa.cpp
endif

ifeq (${MK_USE_KERNEL_UORB},px4)
PROJ_CDEFS += CONFIG_MODULE_UORB
PROJ_CINCDIRS += kernel/module/uorb
CSOURCES += kernel/module/uorb/publication.c
CSOURCES += kernel/module/uorb/publication_multi.c
CSOURCES += kernel/module/uorb/subscription.c
CSOURCES += kernel/module/uorb/subscription_interval.c
CSOURCES += kernel/module/uorb/subscription_callback.c
CSOURCES += kernel/module/uorb/subscription_blocking.c
CSOURCES += kernel/module/uorb/uorb_device_master.c
CSOURCES += kernel/module/uorb/uorb_device_node.c
CSOURCES += kernel/module/uorb/uorb_gnode.c
CSOURCES += kernel/module/uorb/uorb_manager.c
CSOURCES += kernel/module/uorb/uorb.c

ifeq (${MK_TEST_ENABLE},y)
PROJ_CINCDIRS += kernel/module/uorb/test
CSOURCES += kernel/module/uorb/test/uorb_test.c
endif

PROJ_CINCDIRS += build/
CPPSOURCES += $(subst ${SDK_ROOTDIR}/,,$(wildcard ${SDK_ROOTDIR}/build/msg/topics_sources/*cpp))

MK_USE_KERNEL_WORKQUEUE=y
MK_USE_KERNEL_POSIX_PTHREAD:=y
MK_USE_KERNEL_POSIX_SEMAPHORE:=y
MK_USE_KERNEL_HRT:=y
endif

ifeq (${MK_USE_KERNEL_WORKQUEUE},y)
PROJ_CDEFS += CONFIG_MODULE_WORKQUEUE
PROJ_CINCDIRS += kernel/module/common
PROJ_CINCDIRS += kernel/module/ccontainer
PROJ_CINCDIRS += kernel/module/workqueue
CSOURCES += kernel/module/ccontainer/blocking_list.c
CSOURCES += kernel/module/ccontainer/intrusive_queue.c
CSOURCES += kernel/module/ccontainer/intrusive_list.c
CSOURCES += kernel/module/ccontainer/intrusive_sorted_list.c
CSOURCES += kernel/module/workqueue/scheduledworkitem.c
CSOURCES += kernel/module/workqueue/workitem.c
CSOURCES += kernel/module/workqueue/workqueue.c
CSOURCES += kernel/module/workqueue/workqueue_manager.c

ifeq (${MK_TEST_ENABLE},y)
PROJ_CINCDIRS += kernel/module/workqueue/test
CSOURCES += kernel/module/workqueue/test/wqueue_test.c
CSOURCES += kernel/module/workqueue/test/wqueue_scheduled_test.c
CSOURCES += kernel/module/workqueue/test/wqueue_all_test.c
endif

MK_USE_KERNEL_POSIX_PTHREAD:=y
MK_USE_KERNEL_POSIX_SEMAPHORE:=y
MK_USE_KERNEL_HRT:=y
endif

ifeq (${MK_USE_KERNEL_PERF},y)
PROJ_CDEFS += CONFIG_MODULE_PERF
PROJ_CINCDIRS += kernel/module/perf
CSOURCES += kernel/module/perf/perf_counter.c

ifeq (${MK_TEST_ENABLE},y)
CSOURCES += kernel/module/perf/perf_counter_test.c
endif

MK_USE_KERNEL_POSIX_QUEUE:=y
MK_USE_KERNEL_POSIX_PTHREAD:=y
MK_USE_KERNEL_HRT:=y
endif

ifeq (${MK_USE_KERNEL_HRT},y)
PROJ_CDEFS += CONFIG_MODULE_HRT
PROJ_CINCDIRS += kernel/module/hrtimer
PROJ_CINCDIRS += kernel/module/hrtimer/test

ifeq (${MK_TEST_ENABLE},y)
CPPSOURCES += kernel/module/hrtimer/test/hrt_test.cpp
endif

MK_USE_KERNEL_POSIX_QUEUE:=y
MK_USE_KERNEL_POSIX_TIME:=y
endif

#####################################################################
# POSIX 
#####################################################################
ifeq (${MK_USE_KERNEL_POSIX_QUEUE},y)
PROJ_CDEFS += CONFIG_MODULE_KPOSIX_QUEUE
CSOURCES += kernel/module/libc/queue/dq_addafter.c
CSOURCES += kernel/module/libc/queue/dq_addbefore.c
CSOURCES += kernel/module/libc/queue/dq_addfirst.c
CSOURCES += kernel/module/libc/queue/dq_addlast.c
CSOURCES += kernel/module/libc/queue/dq_cat.c
CSOURCES += kernel/module/libc/queue/dq_count.c
CSOURCES += kernel/module/libc/queue/dq_rem.c
CSOURCES += kernel/module/libc/queue/dq_remfirst.c
CSOURCES += kernel/module/libc/queue/dq_remlast.c
CSOURCES += kernel/module/libc/queue/sq_addafter.c
CSOURCES += kernel/module/libc/queue/sq_addfirst.c
CSOURCES += kernel/module/libc/queue/sq_addlast.c
CSOURCES += kernel/module/libc/queue/sq_cat.c
CSOURCES += kernel/module/libc/queue/sq_count.c
CSOURCES += kernel/module/libc/queue/sq_rem.c
CSOURCES += kernel/module/libc/queue/sq_remafter.c
CSOURCES += kernel/module/libc/queue/sq_remfirst.c
CSOURCES += kernel/module/libc/queue/sq_remlast.c
endif

ifeq (${MK_USE_KERNEL_POSIX_MQUEUE},y)
PROJ_CDEFS += CONFIG_MODULE_KPOSIX_MQUEUE
CSOURCES += kernel/module/libc/mqueue/mq_close.c
CSOURCES += kernel/module/libc/mqueue/mq_getattr.c
CSOURCES += kernel/module/libc/mqueue/mq_open.c
CSOURCES += kernel/module/libc/mqueue/mq_receive.c
CSOURCES += kernel/module/libc/mqueue/mq_send.c
CSOURCES += kernel/module/libc/mqueue/mq_timedreceive.c
CSOURCES += kernel/module/libc/mqueue/mq_timedsend.c
CSOURCES += kernel/module/libc/mqueue/mq_unlink.c
CSOURCES += kernel/module/libc/mqueue/prv_mqueue.c

ifeq (${MK_TEST_ENABLE},y)
CSOURCES += kernel/module/libc/tests/klibc_mq_test.c
endif

ifneq (${MK_RTOS},frtos)
$(error need config param MK_RTOS to frtos)
endif
MK_USE_KERNEL_POSIX_TIME:=y
endif

ifeq (${MK_USE_KERNEL_POSIX_PTHREAD},y)
PROJ_CDEFS += CONFIG_MODULE_KPOSIX_PTHREAD
CSOURCES += kernel/module/libc/pthread/pthread.c
CSOURCES += kernel/module/libc/pthread/pthread_attr.c
CSOURCES += kernel/module/libc/pthread/pthread_barrier.c
CSOURCES += kernel/module/libc/pthread/pthread_cond.c
CSOURCES += kernel/module/libc/pthread/pthread_mutex.c
CSOURCES += kernel/module/libc/pthread/sched.c

ifeq (${MK_TEST_ENABLE},y)
CSOURCES += kernel/module/libc/tests/klibc_pthread_mutex_test.c
CSOURCES += kernel/module/libc/tests/klibc_pthread_test.c
endif

ifneq (${MK_RTOS},frtos)
$(error need config param MK_RTOS to frtos)
endif
MK_USE_KERNEL_POSIX_TIME:=y
endif

ifeq (${MK_USE_KERNEL_POSIX_SEMAPHORE},y)
PROJ_CDEFS += CONFIG_MODULE_KPOSIX_SEMAPHORE
CSOURCES += kernel/module/libc/semaphore/sem_destroy.c
CSOURCES += kernel/module/libc/semaphore/sem_getvalue.c
CSOURCES += kernel/module/libc/semaphore/sem_init.c
CSOURCES += kernel/module/libc/semaphore/sem_post.c
CSOURCES += kernel/module/libc/semaphore/sem_timedwait.c
CSOURCES += kernel/module/libc/semaphore/sem_trywait.c
CSOURCES += kernel/module/libc/semaphore/sem_wait.c

ifeq (${MK_TEST_ENABLE},y)
CSOURCES += kernel/module/libc/tests/klibc_sem_test.c
endif

ifneq (${MK_RTOS},frtos)
$(error need config param MK_RTOS to frtos)
endif
MK_USE_KERNEL_POSIX_TIME:=y
endif

ifeq (${MK_USE_KERNEL_POSIX_TIMER},y)
PROJ_CDEFS += CONFIG_MODULE_KPOSIX_TIMER
CSOURCES += kernel/module/libc/timer/prv_timer.c
CSOURCES += kernel/module/libc/timer/timer_create.c
CSOURCES += kernel/module/libc/timer/timer_delete.c
CSOURCES += kernel/module/libc/timer/timer_getoverrun.c
CSOURCES += kernel/module/libc/timer/timer_gettime.c
CSOURCES += kernel/module/libc/timer/timer_settime.c

ifeq (${MK_TEST_ENABLE},y)
CSOURCES += kernel/module/libc/tests/klibc_timer_test.c
endif

ifneq (${MK_RTOS},frtos)
$(error need config param MK_RTOS to frtos)
endif
MK_USE_KERNEL_POSIX_TIME:=y
endif

ifeq (${MK_USE_KERNEL_POSIX_TIME},y)
PROJ_CDEFS += CONFIG_MODULE_KPOSIX_TIME
CSOURCES += kernel/module/libc/time/clock.c
CSOURCES += kernel/module/libc/time/clock_getres.c
CSOURCES += kernel/module/libc/time/clock_gettime.c
CSOURCES += kernel/module/libc/time/clock_nanosleep.c
CSOURCES += kernel/module/libc/time/clock_settime.c
CSOURCES += kernel/module/libc/time/lib_nanosleep.c
CSOURCES += kernel/module/libc/unistd/lib_sleep.c
CSOURCES += kernel/module/libc/unistd/lib_usleep.c

ifeq (${MK_TEST_ENABLE},y)
CSOURCES += kernel/module/libc/tests/klibc_clock_test.c
endif

ifneq (${MK_RTOS},frtos)
$(error need config param MK_RTOS to frtos)
endif
endif






