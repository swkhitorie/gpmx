
CPPSOURCES += ${PX4LIBPATH}/lib/conversion/rotation.cpp

ifeq (${CONFIG_TEST},y)
CPPSOURCES += ${PX4LIBPATH}/lib/conversion/RotationTest.cpp
endif
