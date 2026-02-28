#include <px4_platform_common/i2c.h>
#include <px4_arch/i2c_hw_description.h>

constexpr px4_i2c_bus_t px4_i2c_buses[I2C_BUS_MAX_BUS_ITEMS] = {
    initI2CBusInternal(1),
};


// bool board_gpioread(uint32_t pinset)
// {
//     GPIO_TypeDef *port = PIN_STPORT(pinset);
//     uint16_t pin = PIN_STPIN(pinset);

//     return HAL_GPIO_ReadPin(port, 1<<pin);
// }

// void board_gpiowrite(uint32_t pinset, bool value)
// {
//     GPIO_TypeDef *port = PIN_STPORT(pinset);
//     uint16_t pin = PIN_STPIN(pinset);

//     HAL_GPIO_WritePin(port, 1<<pin, value);
// }

// int board_gpiosetevent(uint32_t pinset, bool risingedge, bool fallingedge,
//                        bool event, io_exit_func func, void *arg)
// {
//     GPIO_TypeDef *port = PIN_STPORT(pinset);
//     uint16_t pin = PIN_STPIN(pinset);
//     uint32_t mode;
//     uint32_t pull;

//     if (fallingedge && !risingedge) {

//         mode = IOMODE_IT_FALLING;
//         pull = IO_PULLUP;
//     } else if (!fallingedge && risingedge) {

//         mode = IOMODE_IT_RISING;
//         pull = IO_PULLDOWN;
//     } else if (fallingedge && risingedge) {

//         mode = IOMODE_IT_BOTH;
//         pull = IO_NOPULL;
//     } else {

//         // param error
//     }

//     low_gpio_setup(port, pin, mode, pull, IO_SPEEDHIGH, 0, func, arg, 1);
// }

// struct i2c_master_s *board_i2cbus_initialize(int bus)
// {
//     struct i2c_master_s *dev;
//     switch (bus) {
//     case 1:
//         dev = NULL;
//         break;
//     case 2:
//         dev = NULL;
//         break;
//     }

//     return dev;
// }

// int board_i2cbus_uninitialize(struct i2c_master_s *pdev)
// {
//     (void)pdev;

//     return 0;
// }

// struct spi_dev_s *board_spibus_initialize(int bus)
// {
//     struct spi_dev_s *dev;
//     switch (bus) {
//     case 1:
//         dev = NULL;
//         break;
//     case 2:
//         dev = NULL;
//         break;
//     }

//     return dev;
// }
