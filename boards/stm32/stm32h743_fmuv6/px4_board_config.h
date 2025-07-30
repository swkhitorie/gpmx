#ifndef BOARD_PX4_CONFIG_H_
#define BOARD_PX4_CONFIG_H_

enum board_bus_types {
	BOARD_INVALID_BUS = 0,
	BOARD_SPI_BUS = 1,
	BOARD_I2C_BUS = 2
};

#define board_has_bus(t, b) true

#define PX4_NUMBER_I2C_BUSES    1

#define SPI_BUS_MAX_BUS_ITEMS   2

#endif
