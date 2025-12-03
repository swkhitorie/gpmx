
ifeq (${MK_LIB_EMBED_PRINTF},y)
PROJ_CINCDIRS   +=  libs/printf
CSOURCES   +=  libs/printf/lib_eprintf.c
endif

