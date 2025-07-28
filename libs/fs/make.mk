
ifeq (${MK_USE_FS_FATFS},y)
PROJ_CINCDIRS += libs/fs/fat/
CSOURCES += libs/fs/fat/ff.c
CSOURCES += libs/fs/fat/diskio.c
CSOURCES += libs/fs/fat/ff_drv.c
CSOURCES += libs/fs/fat/ffsystem.c
CSOURCES += libs/fs/fat/ffunicode.c
endif # end with MK_USE_FS_FATFS - y

ifeq (${MK_USE_FS_FATFS},l)
PROJ_CINCDIRS += libs/fs/fat/legacy
CSOURCES += libs/fs/fat/legacy/ff.c
CSOURCES += libs/fs/fat/legacy/diskio.c
CSOURCES += libs/fs/fat/legacy/ff_drv.c
CSOURCES += libs/fs/fat/legacy/option/syscall.c
CSOURCES += libs/fs/fat/legacy/option/unicode.c
endif # end with MK_USE_FS_FATFS - l
