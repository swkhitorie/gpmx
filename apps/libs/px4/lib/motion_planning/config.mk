
CPPSOURCES += ${PX4LIBPATH}/lib/motion_planning/VelocitySmoothing.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/motion_planning/PositionSmoothing.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/motion_planning/ManualVelocitySmoothingXY.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/motion_planning/ManualVelocitySmoothingZ.cpp

ifeq (${CONFIG_TEST},y)
CPPSOURCES += ${PX4LIBPATH}/lib/motion_planning/VelocitySmoothingTest.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/motion_planning/PositionSmoothingTest.cpp
endif