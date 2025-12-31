
ifeq (${MK_USE_CSH},y)
PROJ_CINCDIRS += libs/sh/csh/
CSOURCES += libs/sh/csh/chry_shell.c
CSOURCES += libs/sh/csh/cherryrl/chry_readline.c
endif
