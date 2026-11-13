
PROJ_CINCDIRS += ${GPMPATH}/mm

ifneq (${CONFIG_POSIXRUN_ENABLE},y)
CSOURCES += ${GPMPATH}/mm/mm_malloc.c
CSOURCES += ${GPMPATH}/mm/mm_free.c
endif

ifeq (${CONFIG_MM_RTXMEM},y)
CSOURCES += ${GPMPATH}/mm/rtxmem.c
endif
