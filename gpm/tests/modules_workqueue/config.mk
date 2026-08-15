
ifeq (${CONFIG_MODULE_WORKQUEUE},y)
CSOURCES += ${GPMPATH}/tests/modules_workqueue/wqueue_test.c
CSOURCES += ${GPMPATH}/tests/modules_workqueue/wqueue_scheduled_test.c
CSOURCES += ${GPMPATH}/tests/modules_workqueue/wqueue_all_test.c
endif
