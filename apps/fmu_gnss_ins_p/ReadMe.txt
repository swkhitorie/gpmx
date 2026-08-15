
ulog simulate:
	export replay=~/__workplace/gpmx/tools/ulog_download/ulog_4.ulg

build command: 
	./tools/buildu.sh -j8 -r -a fmu_gnss_ins_p -u

board_sim_topic_filter:
	adc_report						[drivers/adc]
	battery_status					[drivers/osd,smart_battery,uavcan,batt_smbus,rc_input,power_monitor]
	system_power					[drivers/adc]
	power_monitor					[drivers/power_monitor]

	sensor_accel					[lib/drivers/accel]
	sensor_accel_fifo     			[lib/drivers/accel]
	sensor_gyro						[lib/drivers/gyro]
	sensor_gyro_fifo				[lib/drivers/gyro]
	sensor_mag						[lib/drivers/mag]
	sensor_baro						[drivers/barometer]
	sensor_hygrometer				[drivers/hygrometer]
	sensor_optical_flow				[drivers/optical_fllow]
	sensor_uwb						[drivers/uwb]
	sensor_gnss_relative			[drivers/gps]
	sensor_gps						[drivers/gps]
	gps_dump						[drivers/gps]
	gps_inject_data					[drivers/gps]
	satellite_info                  [drivers/gps]
	distance_sensor					[lib/drivers/rangefinder]
	differential_pressure			[drivers/differential_pressure]

	esc_status						[drivers/dshot, uavcan]
	heater_status					[driver/heater]
	input_rc						[driver/rc]
	pps_capture						[drivers/camera_capture,camera_trigger,pps_capture]
	pwm_input						[drivers/pwm_input]
	px4io_status					[drivers/px4io]
	rpm								[drivers/rpm]
	button_event					[libs/button, drivers/safety_button]

	cpuload							[modules/loadmon]
	esc_report
    rc_channels						[modules/rc_update]
	mag_worker_data					[modules/commander(calibration)]

	timesync_status					[lib/timesync]
	sensors_status					[modules/sensors]
	sensor_selection				[modules/ekf2]
	sensors_status_imu				[modules/ekf2]
	telemetry_status				[modules/mavlink]
	radio_status					[modules/mavlink]

