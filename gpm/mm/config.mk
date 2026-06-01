
# ifeq (${MK_USE_MODULE_RTXMEM},y)
PROJ_CDEFS += CONFIG_MODULE_MEMRTX
PROJ_CINCDIRS += ${GPMPATH}/mm
# CSOURCES += ${GPMPATH}/mm/rtxmem.c
CSOURCES += ${GPMPATH}/mm/mm_malloc.c
CSOURCES += ${GPMPATH}/mm/mm_free.c
# EXT_LIBOPTS += \
#     -Wl,--wrap=malloc \
# 	-Wl,--wrap=free
# endif
