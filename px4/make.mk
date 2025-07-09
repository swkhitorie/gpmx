
FR_CINCDIRS += px4/platforms/common/
FR_CINCDIRS += px4/src/
FR_CINCDIRS += px4/src/include/

ifeq (${CONFIG_MK_USE_PX4_TASKS},y)
FR_CPPSOURCES +=  px4/platforms/px4_tasks.cpp
endif

ifeq (${CONFIG_MK_USE_PX4_HRT},y)
FR_CINCDIRS   +=  px4/platforms/hrt/
FR_CSOURCES   +=  px4/platforms/hrt/hrt.c
ifeq (${CONFIG_MK_USE_FR_POSIX},n)
FR_CINCDIRS += px4/platforms/hrt/queue
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


ifeq (${CONFIG_MK_USE_PX4_UORB},y)
#include ${SDK_ROOTDIR}/libs/uorb_msgs/make.mk
FR_CINCDIRS   +=  px4/platforms/common/uorb/include
FR_CINCDIRS   +=  px4/platforms/common/uorb/src
FR_CPPSOURCES +=  px4/platforms/common/uorb/src/device_master.cpp
FR_CPPSOURCES +=  px4/platforms/common/uorb/src/device_node.cpp
FR_CPPSOURCES +=  px4/platforms/common/uorb/src/uorb.cpp
endif

ifeq (${CONFIG_MK_USE_PX4_WORKQUEUE},y)
FR_CPPSOURCES +=  px4/platforms/common/px4_work_queue/ScheduledWorkItem.cpp
FR_CPPSOURCES +=  px4/platforms/common/px4_work_queue/WorkItem.cpp
FR_CPPSOURCES +=  px4/platforms/common/px4_work_queue/WorkItemSingleShot.cpp
FR_CPPSOURCES +=  px4/platforms/common/px4_work_queue/WorkQueue.cpp
FR_CPPSOURCES +=  px4/platforms/common/px4_work_queue/WorkQueueManager.cpp
endif

ifeq (${CONFIG_MK_USE_PX4_LOG},y)
FR_CPPSOURCES +=  px4/platforms/common/px4_log.cpp
endif

ifeq (${CONFIG_MK_USE_PX4_MODULE},y)
FR_CPPSOURCES +=  px4/platforms/common/module.cpp
endif

ifeq (${CONFIG_MK_USE_PX4_PERF_CNTER},y)
FR_CPPSOURCES +=  px4/src/lib/perf/perf_counter.cpp
endif

