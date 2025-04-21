
ifeq (${CONFIG_FR_LIB_PX4_SUPPORT},y)
FR_CSOURCES   +=  libs/gcl/hrt/hrt.c
FR_CPPSOURCES +=  libs/gcl/hrt/px4_tasks.cpp
else

FR_CINCDIRS   +=  libs/gcl
ifeq (${CONFIG_USE_DRV_HRT_INTERNAL},y)
FR_CSOURCES   +=  libs/gcl/hrt/hrt.c

ifeq (${CONFIG_FR_LIB_POSIX},n)
FR_CINCDIRS += libs/gcl/hrt/queue
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
endif

endif

endif
