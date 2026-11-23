
# CONFIG_MODULES_EKF2
# CONFIG_EKF2_MULTI_INSTANCE       # EKF2 support multiple instances and selector.
# CONFIG_EKF2_AIRSPEED             # EKF2 airspeed fusion support.
# CONFIG_EKF2_AUXVEL               # EKF2 auxiliary velocity fusion support.
# CONFIG_EKF2_BARO_COMPENSATION    # EKF2 barometer pressure compensation support.
# CONFIG_EKF2_DRAG_FUSION          # EKF2 drag fusion support.
# CONFIG_EKF2_EXTERNAL_VISION      # EKF2 external vision (EV) fusion support.
# CONFIG_EKF2_GNSS_YAW             # EKF2 GNSS yaw fusion support.
# CONFIG_EKF2_OPTICAL_FLOW         # EKF2 optical flow fusion support.
# CONFIG_EKF2_RANGE_FINDER         # EKF2 range finder fusion support.
# CONFIG_EKF2_SIDESLIP             # EKF2 sideslip fusion support.

PARAMETERS_MODULE_LIST += ${SDK_ROOTDIR}/${PX4LIBPATH}/modules/ekf2

ALL_EXT_GROUPS += ekf2
ekf2_OPT := -DMODULE_NAME=\"ekf2\" -fno-associative-math

ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF2.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF2Selector.cpp

ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/baro_height_control.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/bias_estimator.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/control.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/covariance.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/ekf.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/ekf_helper.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/EKFGSF_yaw.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/estimator_interface.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/fake_height_control.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/fake_pos_control.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/gnss_height_control.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/gps_checks.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/gps_control.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/gravity_fusion.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/height_control.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/imu_down_sampler.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/mag_control.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/mag_fusion.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/output_predictor.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/vel_pos_fusion.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/zero_innovation_heading_update.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/zero_velocity_update.cpp

ifeq (${CONFIG_EKF2_AIRSPEED},1)
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/airspeed_fusion.cpp
endif

ifeq (${CONFIG_EKF2_AUXVEL},1)
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/auxvel_fusion.cpp
endif

ifeq (${CONFIG_EKF2_DRAG_FUSION},1)
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/drag_fusion.cpp
endif

ifeq (${CONFIG_EKF2_EXTERNAL_VISION},1)
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/ev_control.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/ev_height_control.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/ev_pos_control.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/ev_vel_control.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/ev_yaw_control.cpp
endif

ifeq (${CONFIG_EKF2_GNSS_YAW},1)
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/gps_yaw_fusion.cpp
endif

ifeq (${CONFIG_EKF2_OPTICAL_FLOW},1)
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/optical_flow_control.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/optflow_fusion.cpp
endif

ifeq (${CONFIG_EKF2_RANGE_FINDER},1)
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/range_finder_consistency_check.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/range_height_control.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/sensor_range_finder.cpp
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/terrain_estimator.cpp
endif

ifeq (${CONFIG_EKF2_SIDESLIP},1)
ekf2_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/sideslip_fusion.cpp
endif

CPPSOURCES += ${ekf2_SRC}

include ${SDK_ROOTDIR}/${PX4LIBPATH}/modules/ekf2/Utility/config.mk

ifeq (${CONFIG_TEST},y)
include ${SDK_ROOTDIR}/${PX4LIBPATH}/modules/ekf2/EKF/config.mk
include ${SDK_ROOTDIR}/${PX4LIBPATH}/modules/ekf2/test/config.mk
endif

ifeq (${CONFIG_MODULE_NSHLIB},y)
MODULE_LIBRARIES += ekf2_module
MOD_MAIN_ekf2_module  := ekf2
MOD_PRIO_ekf2_module  := SCHED_PRIORITY_MAX - 18
MOD_STACK_ekf2_module := 4096
endif
