#include <drv_gpio.h>

#include <drivers/drv_sensor.h>
#include <px4_platform_common/spi.h>
#include <px4_arch/spi_hw_description.h>

constexpr px4_spi_bus_t px4_spi_buses[SPI_BUS_MAX_BUS_ITEMS] = {
    initSPIBus(SPI::Bus::SPIBUS1, {
        initSPIDevice(DRV_IMU_DEVTYPE_MPU6000, SPI::CS{GPIO::PortC, GPIO::Pin2}, SPI::DRDY{GPIO::PortD, GPIO::Pin15}),
        initSPIDevice(DRV_GYR_DEVTYPE_L3GD20, SPI::CS{GPIO::PortC, GPIO::Pin13}, SPI::DRDY{GPIO::PortB, GPIO::Pin10}),
        initSPIDevice(DRV_IMU_DEVTYPE_LSM303D, SPI::CS{GPIO::PortC, GPIO::Pin15}),
        initSPIDevice(DRV_BARO_DEVTYPE_MS5611, SPI::CS{GPIO::PortD, GPIO::Pin7}),
    }, {GPIO::PortE, GPIO::Pin3}),
};

