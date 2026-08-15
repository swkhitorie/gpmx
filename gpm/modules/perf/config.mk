
ifeq (${CONFIG_MODULE_PERF},y)

ifneq (${CONFIG_POSIXRUN_ENABLE},y)
ifneq (${CONFIG_LIBC_PTHREAD}${CONFIG_LIBC_QUEUE},yy)
$(error CONFIG_MODULE_PERF depend on CONFIG_LIBC_PTHREAD, CONFIG_LIBC_QUEUE)
endif
else
ifneq (${CONFIG_LIBC_QUEUE},y)
$(error CONFIG_MODULE_PERF depend on CONFIG_LIBC_QUEUE)
endif
endif

PROJ_CINCDIRS += ${GPMPATH}/modules/perf
CSOURCES += ${GPMPATH}/modules/perf/perf_counter.c
endif
