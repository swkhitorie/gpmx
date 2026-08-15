
PROJ_CINCDIRS += ${GPMPATH}/mm

ifneq (${CONFIG_POSIXRUN_ENABLE},y)
CSOURCES += ${GPMPATH}/mm/mm_malloc.c
CSOURCES += ${GPMPATH}/mm/mm_free.c
endif
