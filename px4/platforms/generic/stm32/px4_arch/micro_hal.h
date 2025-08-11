
#pragma once

#include <board_config.h>

#define PX4_BUS_OFFSET       0                  /* STM buses are 1 based no adjustment needed */

#define px4_i2cbus_initialize(bus_num_1based)   board_i2cbus_initialize(bus_num_1based)
#define px4_i2cbus_uninitialize(pdev)           board_i2cbus_uninitialize(pdev)

#define px4_spibus_initialize(bus_num_1based)   board_spibus_initialize(bus_num_1based)

#define px4_arch_gpioread(pinset)               board_gpioread(pinset)
#define px4_arch_gpiowrite(pinset, value)       board_gpiowrite(pinset, value)
#define px4_arch_gpiosetevent(pinset,r,f,e,fp,a)  board_gpiosetevent(pinset,r,f,e,fp,a)

