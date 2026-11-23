
PROJ_CINCDIRS += ${PX4LIBPATH}/modules/ekf2

EXT_CDEFS += TEST_DATA_PATH=\"${PX4LIBPATH}/modules/ekf2/test\"

include ${SDK_ROOTDIR}/${PX4LIBPATH}/modules/ekf2/test/sensor_simulator/config.mk
include ${SDK_ROOTDIR}/${PX4LIBPATH}/modules/ekf2/test/test_helper/config.mk

CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_accelerometer.cpp
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_airspeed_fusion_generated.cpp 
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_airspeed.cpp
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_basics.cpp
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_covariance_prediction_generated.cpp 
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_externalVision.cpp 
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_flow.cpp  
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_opt_flow_fusion_generated.cpp 
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_fusionLogic.cpp  
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_gps.cpp  
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_gps_yaw.cpp
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_gnss_yaw_generated.cpp 
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_height_fusion.cpp  
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_imuSampling.cpp
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_initialization.cpp
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_mag.cpp
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_mag_3d_fusion_generated.cpp 
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_mag_declination_generated.cpp 
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_measurementSampling.cpp
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_ringbuffer.cpp
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_sideslip_fusion_generated.cpp 
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_terrain_estimator.cpp
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_utils.cpp
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_withReplayData.cpp
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_yaw_estimator.cpp  
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_yaw_estimator_generated.cpp 
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_yaw_fusion_generated.cpp 
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_SensorRangeFinder.cpp
CPPSOURCES += ${PX4LIBPATH}/modules/ekf2/test/test_EKF_drag_fusion.cpp
