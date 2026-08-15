
ifeq (${CONFIG_MODULE_NSHLIB},y)

ifneq (${CONFIG_MODULE_BUILTIN}${CONFIG_MODULE_READLINE},yy)
$(error CONFIG_MODULE_NSHLIB depend on CONFIG_MODULE_BUILTIN, CONFIG_MODULE_READLINE)
endif

PROJ_CINCDIRS += ${GPMPATH}/modules/nshlib
CSOURCES += ${GPMPATH}/modules/nshlib/nsh_builtin.c
CSOURCES += ${GPMPATH}/modules/nshlib/nsh_command.c
CSOURCES += ${GPMPATH}/modules/nshlib/nsh_console.c
CSOURCES += ${GPMPATH}/modules/nshlib/nsh_consolemain.c
CSOURCES += ${GPMPATH}/modules/nshlib/nsh_envcmds.c
CSOURCES += ${GPMPATH}/modules/nshlib/nsh_parse.c
CSOURCES += ${GPMPATH}/modules/nshlib/nsh_script.c
CSOURCES += ${GPMPATH}/modules/nshlib/nsh_session.c
CSOURCES += ${GPMPATH}/modules/nshlib/nsh_test.c
CSOURCES += ${GPMPATH}/modules/nshlib/nsh_vars.c
endif

