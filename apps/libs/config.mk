
PLIBS = apps

PROJ_CINCDIRS += ${PLIBS}
PROJ_CINCDIRS += ${PLIBS}/libs
PROJ_CINCDIRS += ${PLIBS}/libs/matrix

include ${PLIBS}/libs/platform/config.mk
include ${PLIBS}/libs/drivers/config.mk

include ${PLIBS}/libs/airspeed/config.mk
include ${PLIBS}/libs/controllib/config.mk
include ${PLIBS}/libs/conversion/config.mk
include ${PLIBS}/libs/geo/config.mk
include ${PLIBS}/libs/systemlib/config.mk
include ${PLIBS}/libs/terrain_estimation/config.mk
include ${PLIBS}/libs/timesync/config.mk
include ${PLIBS}/libs/world_magnetic_model/config.mk
