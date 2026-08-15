#include <board_config.h>
#include <stdio.h>

#include <gpm/drv_hrt.h>

#include <workqueue_manager.h>

#include "uorb_common.h"
#include "uorb_manager.h"

#include "drivers/drv_sensor.h"

#if defined(BOARD_PXBOARD_EBFV2)
#include "drivers/imu/mpu6050/MPU6050.hpp"
#endif

void sensors_group_start(int argc, char **argv)
{
#if defined(BOARD_PXBOARD_EBFV2)
    static MPU6050 *_imu;
    _imu = new MPU6050(DRV_IMU_DEVTYPE_MPU6500, 
        ROTATION_NONE, "mpu6050", 1, 0x68, 400000, 
        DRV_IMU_DEVTYPE_MPU6500);
    _imu->init();
#endif

}

extern "C" void main_root(void *p)
{
    workqueue_manager_start();
    uorb_manager_initialize();

    sensors_group_start(0, nullptr);

    vTaskDelete(NULL);
}
