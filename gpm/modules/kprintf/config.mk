ifeq (${MK_USE_MODULE_KPRINTF},y)
PROJ_CDEFS += CONFIG_MODULE_KPRINTF
PROJ_CINCDIRS += ${GPMPATH}/modules/kprintf
CSOURCES += ${GPMPATH}/modules/kprintf/kprintf.c
endif