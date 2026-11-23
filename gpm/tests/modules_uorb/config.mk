
ifeq (${CONFIG_MODULE_UORB},y)
CSOURCES += ${GPMPATH}/tests/modules_uorb/uorb_test.c
CPPSOURCES += ${GPMPATH}/tests/modules_uorb/uorb_cpp_test.cpp
endif
