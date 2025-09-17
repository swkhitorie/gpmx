################################################################################
#
# Cortex-M3 configuration file
#
################################################################################

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
