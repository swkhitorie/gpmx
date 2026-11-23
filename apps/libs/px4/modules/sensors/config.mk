
include ${SDK_ROOTDIR}/${PX4LIBPATH}/modules/sensors/data_validator/config.mk
include ${SDK_ROOTDIR}/${PX4LIBPATH}/modules/sensors/vehicle_imu/config.mk

PARAMETERS_MODULE_LIST += ${SDK_ROOTDIR}/${PX4LIBPATH}/modules/sensors
PARAMETERS_MODULE_CFGFILES += ${SDK_ROOTDIR}/${PX4LIBPATH}/modules/sensors/module.yaml

ifeq (${CONFIG_SENSORS_VEHICLE_ACCELERATION},1)
include ${SDK_ROOTDIR}/${PX4LIBPATH}/modules/sensors/vehicle_acceleration/config.mk
endif

ifeq (${CONFIG_SENSORS_VEHICLE_AIR_DATA},1)
include ${SDK_ROOTDIR}/${PX4LIBPATH}/modules/sensors/vehicle_air_data/config.mk
endif

ifeq (${CONFIG_SENSORS_VEHICLE_ANGULAR_VELOCITY},1)
include ${SDK_ROOTDIR}/${PX4LIBPATH}/modules/sensors/vehicle_angular_velocity/config.mk
endif

ifeq (${CONFIG_SENSORS_VEHICLE_GPS_POSITION},1)
include ${SDK_ROOTDIR}/${PX4LIBPATH}/modules/sensors/vehicle_gps_position/config.mk
endif

ifeq (${CONFIG_SENSORS_VEHICLE_MAGNETOMETER},1)
include ${SDK_ROOTDIR}/${PX4LIBPATH}/modules/sensors/vehicle_magnetometer/config.mk
endif

ifeq (${CONFIG_SENSORS_VEHICLE_OPTICAL_FLOW},1)
include ${SDK_ROOTDIR}/${PX4LIBPATH}/modules/sensors/vehicle_optical_flow/config.mk
endif

PROJ_CINCDIRS += ${PX4LIBPATH}/modules/sensors

ALL_EXT_GROUPS += sensors
sensors_OPT := -DMODULE_NAME=\"sensors\"
sensors_SRC += ${PX4LIBPATH}/modules/sensors/sensors.cpp
sensors_SRC += ${PX4LIBPATH}/modules/sensors/voted_sensors_update.cpp
CPPSOURCES += ${sensors_SRC}

ifeq (${CONFIG_MODULE_NSHLIB},y)
MODULE_LIBRARIES += sensors_module
MOD_MAIN_sensors_module  := sensors
MOD_PRIO_sensors_module  := SCHED_PRIORITY_MAX - 18
MOD_STACK_sensors_module := 4096
endif

