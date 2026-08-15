
ifeq (${CONFIG_MODULE_READLINE},y)

ifneq (${CONFIG_POSIXRUN_ENABLE},y)
ifneq (${CONFIG_LIBC_PTHREAD},y)
$(error CONFIG_MODULE_READLINE depend on CONFIG_LIBC_PTHREAD)
endif
endif

PROJ_CINCDIRS += ${GPMPATH}/modules/readline
CSOURCES += ${GPMPATH}/modules/readline/readline_fd.c
CSOURCES += ${GPMPATH}/modules/readline/readline_common.c
endif
