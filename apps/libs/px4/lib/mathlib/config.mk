
CPPSOURCES += ${PX4LIBPATH}/lib/mathlib/math/test/test.cpp

ifeq (${CONFIG_TEST},y)
CPPSOURCES += ${PX4LIBPATH}/lib/mathlib/math/FunctionsTest.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/mathlib/math/WelfordMeanTest.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/mathlib/math/WelfordMeanVectorTest.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/mathlib/math/MaxDistanceToCircleTest.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/mathlib/math/test/LowPassFilter2pVector3fTest.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/mathlib/math/test/AlphaFilterTest.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/mathlib/math/test/MedianFilterTest.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/mathlib/math/test/NotchFilterTest.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/mathlib/math/test/second_order_reference_model_test.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/mathlib/math/test/UtilitiesTest.cpp
endif

