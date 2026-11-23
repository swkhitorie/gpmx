
ifeq (${CONFIG_MODULE_WORKQUEUE},y)
CSOURCES += ${GPMPATH}/tests/modules_workqueue/wqueue_test.c
CSOURCES += ${GPMPATH}/tests/modules_workqueue/wqueue_scheduled_test.c
CSOURCES += ${GPMPATH}/tests/modules_workqueue/wqueue_all_test.c
CPPSOURCES += ${GPMPATH}/tests/modules_workqueue/workqueue_cpp_test.cpp
endif
