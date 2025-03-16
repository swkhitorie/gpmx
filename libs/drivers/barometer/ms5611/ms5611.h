#pragma once

#include "board_config.h"

#include <string.h>

#if defined(CONFIG_I2C)
# include <drivers/device/i2c.h>
#endif // CONFIG_I2C

#include <drivers/device/device.h>
#include <drivers/device/spi.h>
#include <lib/cdev/CDev.hpp>
#include <px4_platform_common/px4_work_queue/ScheduledWorkItem.hpp>
#include <systemlib/err.h>

#define MS5611_ADDR_RESET_CMD			0x1E	/* write to this address to reset chip */
#define MS5611_ADDR_PROM_SETUP			0xA0	/* address of 8x 2 bytes factory and calibration data */

/* interface ioctls */
#define IOCTL_RESET			2
#define IOCTL_MEASURE			3

#define MS5611_ADDRESS_1		0x76	/* address select pins pulled high */
#define MS5611_ADDRESS_2		0x77    /* address select pins pulled low */

namespace ms5611
{

/**
 * Calibration PROM as reported by the device.
 */
#pragma pack(push,1)
struct prom_s {
    uint16_t factory_setup;
    uint16_t c1_pressure_sens;
    uint16_t c2_pressure_offset;
    uint16_t c3_temp_coeff_pres_sens;
    uint16_t c4_temp_coeff_pres_offset;
    uint16_t c5_reference_temp;
    uint16_t c6_temp_coeff_temp;
    uint16_t serial_and_crc;
};

/**
 * Grody hack for crc4()
 */
union prom_u {
    uint16_t c[8];
    prom_s s;
};
#pragma pack(pop)

extern bool crc4(uint16_t *n_prom);

} /* namespace */

