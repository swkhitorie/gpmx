################################################################################
#
# Cortex-M4 configuration file
#
################################################################################

ifeq (${TC_NAME},armcc)

CORE_ASMOPTS:=\
  --cpu=Cortex-M4.fp.sp             \
  --apcs=interwork

CORE_COPTS:=\
  --cpu=Cortex-M4.fp.sp             \
  --apcs=interwork

CORE_LIBOPTS:=\
  --cpu=Cortex-M4.fp.sp

endif # TC_NAME - arm cc

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

endif # TC_NAME - arm clang

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
  
endif # TC_NAME - gae
