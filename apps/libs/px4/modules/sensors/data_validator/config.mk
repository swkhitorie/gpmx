
sensors_SRC += ${PX4LIBPATH}/modules/sensors/data_validator/DataValidator.cpp
sensors_SRC += ${PX4LIBPATH}/modules/sensors/data_validator/DataValidatorGroup.cpp

ifeq (${CONFIG_TEST},y)
# CPPSOURCES += ${PX4LIBPATH}/modules/sensors/data_validator/tests/tests_common.cpp
# CPPSOURCES += ${PX4LIBPATH}/modules/sensors/data_validator/tests/test_data_validator.cpp
# CPPSOURCES += ${PX4LIBPATH}/modules/sensors/data_validator/tests/test_data_validator_group.cpp
endif
