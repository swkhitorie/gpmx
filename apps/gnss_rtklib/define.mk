
BUILD_COMPILE_OPTIMIZE:=-O1
BUILD_C_STANDARD:=gnu11
BUILD_CXX_STANDARD:=gnu++11

PROJ_CDEFS += CONFIG_APP_ROOTDIR="${SDK_ROOTDIR}${APP_PROJ_DIR}"

PROJ_CINCDIRS += ${APP_PROJ_DIR}
CPPSOURCES += ${APP_PROJ_DIR}/app_main.cpp

PROJ_CINCDIRS += gpm/mm
CSOURCES += gpm/mm/rtxmem.c

CSOURCES += ${APP_PROJ_DIR}/rtk_server.c

PROJ_CINCDIRS += apps/libs/rtklib
# CSOURCES += apps/libs/rtklib/datum.c
CSOURCES += apps/libs/rtklib/ephemeris.c
CSOURCES += apps/libs/rtklib/geoid.c
# CSOURCES += apps/libs/rtklib/gis.c
# CSOURCES += apps/libs/rtklib/ionex.c
CSOURCES += apps/libs/rtklib/lambda.c
# CSOURCES += apps/libs/rtklib/options.c
CSOURCES += apps/libs/rtklib/pntpos.c
CSOURCES += apps/libs/rtklib/postpos.c
# CSOURCES += apps/libs/rtklib/ppp_ar.c
# CSOURCES += apps/libs/rtklib/ppp.c
# CSOURCES += apps/libs/rtklib/preceph.c
CSOURCES += apps/libs/rtklib/rtcm.c
CSOURCES += apps/libs/rtklib/rtcm3.c
# CSOURCES += apps/libs/rtklib/rtcm3e.c
CSOURCES += apps/libs/rtklib/rtkcmn.c
CSOURCES += apps/libs/rtklib/rtkpos.c
CSOURCES += apps/libs/rtklib/rtksvr.c
# CSOURCES += apps/libs/rtklib/sbas.c
CSOURCES += apps/libs/rtklib/solution.c
CSOURCES += apps/libs/rtklib/stream.c
# CSOURCES += apps/libs/rtklib/tides.c
# CSOURCES += apps/libs/rtklib/tle.c
