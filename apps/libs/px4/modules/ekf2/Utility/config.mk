
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/Utility/PreFlightChecker.cpp

ifeq (${CONFIG_TEST},y)
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/Utility/PreFlightCheckerTest.cpp
endif
