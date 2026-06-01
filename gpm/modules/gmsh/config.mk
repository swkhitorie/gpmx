ifeq (${MK_USE_MODULE_GPMSHELL},y)
PROJ_CDEFS += CONFIG_MODULE_GPMSHELL
PROJ_CINCDIRS += ${GPMPATH}/modules/gmsh
CSOURCES += ${GPMPATH}/modules/gmsh/msh.c
CSOURCES += ${GPMPATH}/modules/gmsh/shell.c
CSOURCES += ${GPMPATH}/modules/gmsh/cmd.c
endif
