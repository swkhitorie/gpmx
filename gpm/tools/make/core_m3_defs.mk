################################################################################
#
# Cortex-M3 configuration file
#
################################################################################

VALID_FPU_OPTIONS := none
TC_FPU_OPTION := $(strip $(TC_FPU_OPTION))
ifeq ($(filter $(TC_FPU_OPTION),$(VALID_FPU_OPTIONS)),)
  $(error m3-fpu only support 'none', current option->'$(TC_FPU_OPTION)')
endif

####################
#      armcc       #
####################
ifeq (${TC_NAME},armcc)

endif

####################
#    armclang      #
####################
ifeq (${TC_NAME},armclang)

endif

####################
#    gcc-arm       #
####################
ifeq (${TC_NAME},gae)
#   -mfpu=vfp                        
#  -mfloat-abi=soft                 
CORE_ASMOPTS:=\
  -mcpu=cortex-m3                  \
  -mthumb                          \
  -mthumb-interwork

CORE_COPTS:=\
  -mcpu=cortex-m3                  \
  -mthumb                          \
  -mthumb-interwork                \
  -mapcs-frame                     \
  -mapcs-stack-check

CORE_LIBOPTS:=\
  -mcpu=cortex-m3                  \
  -mthumb                          \
  -mthumb-interwork

endif
