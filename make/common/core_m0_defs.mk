################################################################################
#
# Cortex-M0 configuration file
#
################################################################################

ifeq (${TC_NAME},armcc)


endif # TC_NAME - arm cc

ifeq (${TC_NAME},armclang)



endif # TC_NAME - arm clang

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

endif # TC_NAME - gae
