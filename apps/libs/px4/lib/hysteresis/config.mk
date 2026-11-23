
CPPSOURCES += ${PX4LIBPATH}/lib/hysteresis/hysteresis.cpp

ifeq (${CONFIG_TEST},y)
CPPSOURCES += ${PX4LIBPATH}/lib/hysteresis/HysteresisTest.cpp
endif
