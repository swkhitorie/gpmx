################################################################################
#
# Cortex-M0 configuration file
#
################################################################################

VALID_FPU_OPTIONS := none
TC_FPU_OPTION := $(strip $(TC_FPU_OPTION))
ifeq ($(filter $(TC_FPU_OPTION),$(VALID_FPU_OPTIONS)),)
  $(error m0-fpu only support 'none', current option->'$(TC_FPU_OPTION)')
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
CORE_ASMOPTS:=\
  -mcpu=cortex-m0plus              \
  -mthumb                          \
  -mthumb-interwork

CORE_COPTS:=\
  -mcpu=cortex-m0plus              \
  -mfloat-abi=soft                 \
  -mthumb                          \
  -mthumb-interwork                \
  -mapcs-frame                     \
  -mapcs-stack-check

CORE_LIBOPTS:=\
  -mcpu=cortex-m0plus              \
  -mfloat-abi=soft                 \
  -mthumb                          \
  -mthumb-interwork

endif
