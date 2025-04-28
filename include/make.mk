
FR_CINCDIRS += include/dev

ifeq (${CONFIG_MK_USE_FREERTOS},y)
FR_CINCDIRS += include/fr
endif

ifeq (${CONFIG_MK_USE_FR_POSIX},y)
FR_CINCDIRS += include
endif
