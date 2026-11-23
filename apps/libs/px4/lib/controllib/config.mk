
CPPSOURCES += ${PX4LIBPATH}/lib/controllib/block/Block.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/controllib/block/BlockParam.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/controllib/BlockDerivative.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/controllib/BlockHighPass.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/controllib/BlockIntegral.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/controllib/BlockIntegralTrap.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/controllib/BlockLimit.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/controllib/BlockLimitSym.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/controllib/BlockLowPass.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/controllib/BlockLowPass2.cpp

ifeq (${CONFIG_TEST},y)
CPPSOURCES += ${PX4LIBPATH}/lib/controllib/test/blocks.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/controllib/test/controllib_test_main.cpp
endif
