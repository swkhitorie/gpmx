
PROJ_CINCDIRS += include/

ifeq (${MK_USE_FREERTOS},y)
PROJ_CINCDIRS += include/frtos
endif

ifeq (${MK_USE_FR_POSIX},y)
PROJ_CINCDIRS += include/posix
endif
