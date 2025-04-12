
ifeq (${CONFIG_FR_LIB_PX4_SUPPORT},y)
FR_CSOURCES   +=  libs/gcl/hrt/hrt.c
FR_CPPSOURCES +=  libs/gcl/hrt/px4_tasks.cpp
else

FR_CINCDIRS   +=  libs/gcl
ifeq (${CONFIG_USE_DRV_HRT_INTERNAL},y)
FR_CSOURCES   +=  libs/gcl/hrt/hrt.c
endif

endif
