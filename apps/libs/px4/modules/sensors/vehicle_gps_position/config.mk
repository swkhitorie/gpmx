

sensors_SRC += ${PX4LIBPATH}/modules/sensors/vehicle_gps_position/gps_blending.cpp
sensors_SRC += ${PX4LIBPATH}/modules/sensors/vehicle_gps_position/VehicleGPSPosition.cpp

PARAMETERS_MODULE_LIST += ${SDK_ROOTDIR}/${PX4LIBPATH}/modules/sensors/vehicle_gps_position

ifeq (${CONFIG_TEST},y)
CPPSOURCES += ${PX4LIBPATH}/modules/sensors/vehicle_gps_position/gps_blending_test.cpp
endif
