################################################################################
#
# Cortex-M4 configuration file
#
################################################################################

VALID_FPU_OPTIONS := none sp
TC_FPU_OPTION := $(strip $(TC_FPU_OPTION))
ifeq ($(filter $(TC_FPU_OPTION),$(VALID_FPU_OPTIONS)),)
  $(error m4-fpu only support 'sp', 'none', current option->'$(TC_FPU_OPTION)')
endif

########################################
#              armcc                   #
########################################
ifeq (${TC_NAME},armcc)

ifeq (${TC_FPU_OPTION},none)
TC_FPU_OPTS:= --cpu=Cortex-M4
endif
ifeq (${TC_FPU_OPTION},sp)
TC_FPU_OPTS:= --cpu=Cortex-M4.fp.sp
endif

CORE_COPTS := ${TC_FPU_OPTS}
CORE_COPTS += --apcs=interwork

CORE_ASMOPTS := ${TC_FPU_OPTS}
CORE_ASMOPTS += --apcs=interwork

CORE_LIBOPTS := ${TC_FPU_OPTS}
endif

########################################
#              armclang                #
########################################
ifeq (${TC_NAME},armclang)

ifeq (${TC_FPU_OPTION},none)
TC_FPU_OPTS:= -mfpu=none -mfloat-abi=soft
TC_FPU_OPTS2:= --cpu=Cortex-M4
endif
ifeq (${TC_FPU_OPTION},sp)
TC_FPU_OPTS:= -mfpu=fpv4-sp-d16 -mfloat-abi=hard
TC_FPU_OPTS2:= --cpu=Cortex-M4.fp.sp
endif

CORE_COPTS := -mcpu=cortex-m4
CORE_COPTS += ${TC_FPU_OPTS}

CORE_ASMOPTS := -mcpu=cortex-m4
CORE_ASMOPTS += ${TC_FPU_OPTS}
CORE_ASMOPTS += -g

CORE_LIBOPTS := ${TC_FPU_OPTS2}
endif

########################################
#              gcc-arm                 #
########################################
ifeq (${TC_NAME},gae)

ifeq (${TC_FPU_OPTION},none)
TC_FPU_OPTS:= -mfpu=none -mfloat-abi=soft
endif
ifeq (${TC_FPU_OPTION},sp)
TC_FPU_OPTS:= -mfpu=fpv4-sp-d16 -mfloat-abi=hard
endif

CORE_COPTS := -mcpu=cortex-m4
CORE_COPTS += ${TC_FPU_OPTS}
CORE_COPTS += -mthumb
CORE_COPTS += -mthumb-interwork 
CORE_COPTS += -mapcs-frame 
CORE_COPTS += -mapcs-stack-check

CORE_ASMOPTS := -mcpu=cortex-m4
CORE_ASMOPTS += -mthumb
CORE_ASMOPTS += -mthumb-interwork

CORE_LIBOPTS := -mcpu=cortex-m4
CORE_LIBOPTS += ${TC_FPU_OPTS}
CORE_LIBOPTS += -mthumb
CORE_LIBOPTS += -mthumb-interwork
endif
