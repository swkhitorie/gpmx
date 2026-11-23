
ALL_EXT_GROUPS += ecl_EKF

ecl_EKF_OPT := -fno-associative-math
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/baro_height_control.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/bias_estimator.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/control.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/covariance.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/ekf.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/ekf_helper.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/EKFGSF_yaw.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/estimator_interface.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/fake_height_control.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/fake_pos_control.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/gnss_height_control.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/gps_checks.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/gps_control.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/gravity_fusion.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/height_control.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/imu_down_sampler.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/mag_control.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/mag_fusion.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/output_predictor.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/vel_pos_fusion.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/zero_innovation_heading_update.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/zero_velocity_update.cpp

ifneq (${CONFIG_EKF2_AIRSPEED},n)
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/airspeed_fusion.cpp
endif

ifneq (${CONFIG_EKF2_AUXVEL},n)
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/auxvel_fusion.cpp
endif

ifneq (${CONFIG_EKF2_DRAG_FUSION},n)
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/drag_fusion.cpp
endif

ifneq (${CONFIG_EKF2_EXTERNAL_VISION},n)
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/ev_control.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/ev_height_control.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/ev_pos_control.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/ev_vel_control.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/ev_yaw_control.cpp
endif

ifneq (${CONFIG_EKF2_GNSS_YAW},n)
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/gps_yaw_fusion.cpp
endif

ifneq (${CONFIG_EKF2_OPTICAL_FLOW},n)
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/optical_flow_control.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/optflow_fusion.cpp
endif

ifneq (${CONFIG_EKF2_RANGE_FINDER},n)
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/range_finder_consistency_check.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/range_height_control.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/sensor_range_finder.cpp
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/terrain_estimator.cpp
endif

ifneq (${CONFIG_EKF2_SIDESLIP},n)
ecl_EKF_SRC += ${PX4LIBPATH}/modules/ekf2/EKF/sideslip_fusion.cpp
endif
