
ifeq (${CONFIG_MODULE_BUILTIN},y)
CSOURCES += ${GPMPATH}/tests/modules_nsh/nshlib_test.c

MODULE_LIBRARIES += test1_built
MOD_MAIN_test1_built  := test1
MOD_PRIO_test1_built  := 16
MOD_STACK_test1_built := 4096

MODULE_LIBRARIES += test2_built
MOD_MAIN_test2_built  := test2
MOD_PRIO_test2_built  := 15
MOD_STACK_test2_built := 2048

MODULE_LIBRARIES += test3_built
MOD_MAIN_test3_built  := test3
MOD_PRIO_test3_built  := 14
MOD_STACK_test3_built := 1024

endif
