
FR_CINCDIRS += include/

ifeq (${CONFIG_MK_USE_FREERTOS},y)
FR_CINCDIRS += include/frtos
endif

ifeq (${CONFIG_MK_USE_FR_POSIX},y)
FR_CINCDIRS += include/posix
endif
