
ifeq (${MK_USE_FS_FATFS},y)
PROJ_CINCDIRS += libs/fs/fat/
CSOURCES += libs/fs/fat/ff.c
CSOURCES += libs/fs/fat/diskio.c
CSOURCES += libs/fs/fat/ff_drv.c
CSOURCES += libs/fs/fat/ffsystem.c
CSOURCES += libs/fs/fat/ffunicode.c
endif

