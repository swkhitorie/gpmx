
ifeq (${MK_TEST_ENABLE},y)
ifeq (${MK_USE_KERNEL_PERF},y)
CSOURCES += ${GPMPATH}/tests/perf/perf_counter_test.c
endif
endif
