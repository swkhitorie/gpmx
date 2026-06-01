
ifeq (${MK_TEST_ENABLE},y)
ifeq (${MK_USE_KERNEL_UORB},y)
CSOURCES += ${GPMPATH}/tests/uorb/uorb_test.c
endif
endif

