
CPPSOURCES += ${PX4LIBPATH}/lib/geo/geo.cpp

ifeq (${CONFIG_TEST},y)
CPPSOURCES += ${PX4LIBPATH}/lib/geo/test_geo.cpp
endif
