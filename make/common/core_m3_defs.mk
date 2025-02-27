################################################################################
#
# Cortex-M3 configuration file
#
################################################################################

ifeq (${TC_NAME},armcc)


endif # TC_NAME - arm cc

ifeq (${TC_NAME},armclang)



endif # TC_NAME - arm clang

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


endif # TC_NAME - gae
