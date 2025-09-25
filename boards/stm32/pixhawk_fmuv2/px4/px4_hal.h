#ifndef PX4_MICRO_HAL_H_
#define PX4_MICRO_HAL_H_

#include <board_config.h>
#include <device/i2c_master.h>
#include <device/spi.h>

#ifdef __cplusplus
extern "C" {
#endif

bool board_gpioread(uint32_t pinset);

void board_gpiowrite(uint32_t pinset, bool value);

int board_gpiosetevent(uint32_t pinset, bool risingedge, bool fallingedge,
                       bool event, io_exit_func func, void *arg);

struct i2c_master_s *board_i2cbus_initialize(int bus);

struct spi_dev_s *board_spibus_initialize(int bus);

int board_i2cbus_uninitialize(struct i2c_master_s *pdev);

#ifdef __cplusplus
}
#endif

#include <px4_arch/micro_hal.h>
#include <px4_platform_common/board_common.h>
#define PX4_NUMBER_I2C_BUSES    1
#define SPI_BUS_MAX_BUS_ITEMS   2

#endif
