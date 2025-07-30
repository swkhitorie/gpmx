
PROJ_CINCDIRS += px4/platforms/common/
PROJ_CINCDIRS += px4/src/
PROJ_CINCDIRS += px4/src/lib/
PROJ_CINCDIRS += px4/src/include/
PROJ_CINCDIRS += px4/src/modules/

ifeq (${MK_USE_PX4_COMPONENT},y)
CPPSOURCES +=  px4/src/lib/perf/perf_counter.cpp
CPPSOURCES +=  px4/src/lib/cdev/CDev.cpp
CPPSOURCES +=  px4/src/lib/cdev/cdev_platform.cpp

CPPSOURCES +=  px4/platforms/generic/px4_tasks.cpp
CPPSOURCES +=  px4/platforms/common/module.cpp
CPPSOURCES +=  px4/platforms/common/px4_log.cpp
#CSOURCES +=  px4/platforms/common/px4_getopt.c

endif

ifeq (${MK_USE_PX4_I2CSPI_BUSES},y)
CPPSOURCES +=  px4/platforms/common/spix.cpp
CPPSOURCES +=  px4/platforms/common/i2c.cpp
CPPSOURCES +=  px4/platforms/common/i2c_spi_buses.cpp
endif

ifeq (${MK_USE_PX4_HRT},y)
PROJ_CINCDIRS   +=  px4/platforms/generic/stm32
CSOURCES   +=  px4/platforms/generic/stm32/hrt.c
ifeq (${MK_USE_POSIX},n)
PROJ_CINCDIRS += px4/platforms/generic/stm32/queue
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
endif # MK_USE_POSIX
endif # MK_USE_HRT

ifeq (${MK_USE_PX4_WORKQUEUE},y)
CPPSOURCES +=  px4/platforms/common/px4_work_queue/ScheduledWorkItem.cpp
CPPSOURCES +=  px4/platforms/common/px4_work_queue/WorkItem.cpp
CPPSOURCES +=  px4/platforms/common/px4_work_queue/WorkItemSingleShot.cpp
CPPSOURCES +=  px4/platforms/common/px4_work_queue/WorkQueue.cpp
CPPSOURCES +=  px4/platforms/common/px4_work_queue/WorkQueueManager.cpp
endif

ifeq (${MK_USE_PX4_LIB_ALGORITHM},y)
CPPSOURCES +=  px4/src/lib/conversion/rotation.cpp
CPPSOURCES +=  px4/src/lib/controllib/BlockDerivative.cpp
CPPSOURCES +=  px4/src/lib/controllib/BlockHighPass.cpp
CPPSOURCES +=  px4/src/lib/controllib/BlockIntegral.cpp
CPPSOURCES +=  px4/src/lib/controllib/BlockIntegralTrap.cpp
CPPSOURCES +=  px4/src/lib/controllib/BlockLimit.cpp
CPPSOURCES +=  px4/src/lib/controllib/BlockLimitSym.cpp
CPPSOURCES +=  px4/src/lib/controllib/BlockLowPass.cpp
CPPSOURCES +=  px4/src/lib/controllib/BlockLowPass2.cpp

CPPSOURCES +=  px4/src/lib/geo/geo.cpp
CPPSOURCES +=  px4/src/lib/geo/geodesicGrid.cpp

CPPSOURCES +=  px4/src/lib/world_magnetic_model/geo_mag_declination.cpp

CPPSOURCES +=  px4/src/lib/mathlib/math/matrix_alg.cpp
CPPSOURCES +=  px4/src/lib/mathlib/math/filter/LowPassFilter2p.cpp
CPPSOURCES +=  px4/src/lib/mathlib/math/filter/LowPassFilter2pVector3f.cpp
CPPSOURCES +=  px4/src/lib/mathlib/math/filter/NotchFilterTest.cpp
endif

ifeq (${MK_USE_PX4_UORB},y)
PROJ_CINCDIRS += px4/src/modules/uORB
CPPSOURCES +=  px4/src/modules/uORB/Subscription.cpp
CPPSOURCES +=  px4/src/modules/uORB/uORB.cpp
CPPSOURCES +=  px4/src/modules/uORB/uORBDeviceMaster.cpp
CPPSOURCES +=  px4/src/modules/uORB/uORBDeviceNode.cpp
CPPSOURCES +=  px4/src/modules/uORB/uORBMain.cpp
CPPSOURCES +=  px4/src/modules/uORB/uORBManager.cpp
CPPSOURCES +=  px4/src/modules/uORB/uORBUtils.cpp

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


