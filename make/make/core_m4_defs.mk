################################################################################
#
# Cortex-M4 configuration file
#
################################################################################

####################
#      armcc       #
####################
ifeq (${TC_NAME},armcc)

CORE_ASMOPTS:=\
  --cpu=Cortex-M4.fp.sp             \
  --apcs=interwork

CORE_COPTS:=\
  --cpu=Cortex-M4.fp.sp             \
  --apcs=interwork

CORE_LIBOPTS:=\
  --cpu=Cortex-M4.fp.sp

endif

####################
#    armclang      #
####################
ifeq (${TC_NAME},armclang)

CORE_ASMOPTS:=\
  -mcpu=cortex-m4                   \
  -mfpu=fpv4-sp-d16                 \
  -mfloat-abi=hard                  \
  -g

CORE_COPTS:=\
  -mcpu=cortex-m4                   \
  -mfpu=fpv4-sp-d16                 \
  -mfloat-abi=hard

CORE_LIBOPTS:=\
  --cpu=Cortex-M4.fp.sp

endif

####################
#    gcc-arm       #
####################
ifeq (${TC_NAME},gae)

CORE_ASMOPTS:=\
  -mcpu=cortex-m4                  \
  -mthumb                          \
  -mthumb-interwork

CORE_COPTS:=\
  -mcpu=cortex-m4                  \
  -mfpu=fpv4-sp-d16                \
  -mfloat-abi=hard                 \
  -mthumb                          \
  -mthumb-interwork                \
  -mapcs-frame                     \
  -mapcs-stack-check

CORE_LIBOPTS:=\
  -mcpu=cortex-m4                  \
  -mfpu=fpv4-sp-d16                \
  -mfloat-abi=hard                 \
  -mthumb                          \
  -mthumb-interwork

endif
