
PROJ_CINCDIRS += px4/platforms/common/
PROJ_CINCDIRS += px4/src/
PROJ_CINCDIRS += px4/src/include/

ifeq (${MK_USE_PX4_COMPONENT},y)
CPPSOURCES +=  px4/platforms/px4_tasks.cpp
CPPSOURCES +=  px4/platforms/common/module.cpp
CPPSOURCES +=  px4/platforms/common/px4_log.cpp
CPPSOURCES +=  px4/src/lib/perf/perf_counter.cpp
CPPSOURCES +=  px4/src/lib/cdev/CDev.cpp
CPPSOURCES +=  px4/src/lib/cdev/cdev_platform.cpp
endif

ifeq (${MK_USE_PX4_HRT},y)
PROJ_CINCDIRS   +=  px4/platforms/hrt/
CSOURCES   +=  px4/platforms/hrt/hrt.c
ifeq (${MK_USE_FR_POSIX},n)
PROJ_CINCDIRS += px4/platforms/hrt/queue
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
endif # MK_USE_FR_POSIX
endif # MK_USE_HRT

ifeq (${MK_USE_PX4_WORKQUEUE},y)
CPPSOURCES +=  px4/platforms/common/px4_work_queue/ScheduledWorkItem.cpp
CPPSOURCES +=  px4/platforms/common/px4_work_queue/WorkItem.cpp
CPPSOURCES +=  px4/platforms/common/px4_work_queue/WorkItemSingleShot.cpp
CPPSOURCES +=  px4/platforms/common/px4_work_queue/WorkQueue.cpp
CPPSOURCES +=  px4/platforms/common/px4_work_queue/WorkQueueManager.cpp
endif

ifeq (${MK_USE_PX4_UORB},y)
PROJ_CINCDIRS += px4/src/uORB
CPPSOURCES +=  px4/src/uORB/Subscription.cpp
CPPSOURCES +=  px4/src/uORB/uORB.cpp
CPPSOURCES +=  px4/src/uORB/uORBDeviceMaster.cpp
CPPSOURCES +=  px4/src/uORB/uORBDeviceNode.cpp
CPPSOURCES +=  px4/src/uORB/uORBMain.cpp
CPPSOURCES +=  px4/src/uORB/uORBManager.cpp
CPPSOURCES +=  px4/src/uORB/uORBUtils.cpp

PROJ_CINCDIRS   +=  px4/build/
CPPSOURCES += $(subst ${SDK_ROOTDIR}/,,$(wildcard ${SDK_ROOTDIR}/px4/build/msg/topics_sources/*cpp))

endif

ifeq (${MK_USE_PX4_UORB},posix)
PROJ_CINCDIRS   +=  px4/build/
CPPSOURCES += $(subst ${SDK_ROOTDIR}/,,$(wildcard ${SDK_ROOTDIR}/px4/build/msg/topics_sources/*cpp))
PROJ_CINCDIRS   +=  px4/platforms/common/uorb/include
PROJ_CINCDIRS   +=  px4/platforms/common/uorb/src
CPPSOURCES +=  px4/platforms/common/uorb/src/device_master.cpp
CPPSOURCES +=  px4/platforms/common/uorb/src/device_node.cpp
CPPSOURCES +=  px4/platforms/common/uorb/src/uorb.cpp
endif


