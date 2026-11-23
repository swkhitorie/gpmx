
PX4LIBPATH    := apps/libs/px4

PROJ_CINCDIRS += ${PX4LIBPATH}
PROJ_CINCDIRS += ${PX4LIBPATH}/lib
PROJ_CINCDIRS += ${PX4LIBPATH}/lib/matrix
PROJ_CINCDIRS += ${PX4LIBPATH}/lib/platform
PROJ_CINCDIRS += ${PX4LIBPATH}/lib/platform/include
PROJ_CINCDIRS += ${PX4LIBPATH}/modules

include ${SDK_ROOTDIR}/${PX4LIBPATH}/modules/config.mk
include ${SDK_ROOTDIR}/${PX4LIBPATH}/lib/config.mk

