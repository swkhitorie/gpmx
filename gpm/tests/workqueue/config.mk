
ifeq (${MK_TEST_ENABLE},y)
ifeq (${MK_USE_KERNEL_WORKQUEUE},y)
CSOURCES += ${GPMPATH}/tests/workqueue/wqueue_test.c
CSOURCES += ${GPMPATH}/tests/workqueue/wqueue_scheduled_test.c
CSOURCES += ${GPMPATH}/tests/workqueue/wqueue_all_test.c
endif
endif
