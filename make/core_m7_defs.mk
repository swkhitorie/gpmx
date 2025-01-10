################################################################################
#
# Cortex-M7 configuration file
#
################################################################################

ifeq (${TC_NAME},armcc)

CORE_ASMOPTS:=\
  --cpu=Cortex-M7.fp.sp             \
  --apcs=interwork

CORE_COPTS:=\
  --cpu=Cortex-M7.fp.sp             \
  --apcs=interwork

endif # TC_NAME - arm cc 

ifeq (${TC_NAME},armclang)

CORE_ASMOPTS:=\
  -mcpu=cortex-m7                   \
  -mfpu=fpv5-d16                    \
  -mfloat-abi=hard

CORE_COPTS:=\
  -mcpu=cortex-m7                   \
  -mfpu=fpv5-d16                    \
  -mfloat-abi=hard
  
CORE_LIBOPTS:=\
  --cpu Cortex-M7.fp.dp

endif # TC_NAME - arm clang

ifeq (${TC_NAME},gae)

CORE_ASMOPTS:=\
  -mcpu=cortex-m7                  \
  -mthumb                          \
  -mthumb-interwork
  
CORE_COPTS:=\
  -mcpu=cortex-m7                  \
  -mfpu=fpv5-d16                   \
  -mfloat-abi=hard                 \
  -mthumb                          \
  -mthumb-interwork                \
  -mapcs-frame                     \
  -mapcs-stack-check

CORE_LIBOPTS:=\
  -mcpu=cortex-m7                  \
  -mfpu=fpv5-d16                   \
  -mfloat-abi=hard                 \
  -mthumb                          \
  -mthumb-interwork

endif # TC_NAME - gae
