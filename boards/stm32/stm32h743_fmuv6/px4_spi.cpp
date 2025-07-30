#include <drv_gpio.h>

#include <drivers/drv_sensor.h>
#include <px4_platform_common/spi.h>
#include <px4_arch/spi_hw_description.h>

// constexpr px4_spi_bus_t px4_spi_buses[SPI_BUS_MAX_BUS_ITEMS] = {
//     initSPIBus(SPI::Bus::SPIBUS1, {
//         initSPIDevice(DRV_GYR_DEVTYPE_BMI055, SPI::CS{GPIO::PortC, GPIO::Pin14}, SPI::DRDY{GPIO::PortE, GPIO::Pin5}),
//         initSPIDevice(DRV_ACC_DEVTYPE_BMI055, SPI::CS{GPIO::PortC, GPIO::Pin15}, SPI::DRDY{GPIO::PortE, GPIO::Pin4}),
//         initSPIDevice(DRV_IMU_DEVTYPE_ICM42688P, SPI::CS{GPIO::PortC, GPIO::Pin13}, SPI::DRDY{GPIO::PortE, GPIO::Pin6}),
//     }, {GPIO::PortB, GPIO::Pin2}),
// };

