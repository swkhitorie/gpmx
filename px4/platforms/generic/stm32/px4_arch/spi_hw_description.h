
// #pragma once

// #include <px4_arch/hw_description.h>
// #include <px4_platform_common/spi.h>

// static inline constexpr px4_spi_bus_device_t initSPIDevice(
//     uint32_t devid, 
//     SPI::CS cs_gpio, 
//     SPI::DRDY drdy_gpio = {})
// {
// 	px4_spi_bus_device_t ret{};
// 	ret.cs_gpio = stm32_get_pinnode(getGPIOPort(cs_gpio.port), getGPIOPin(cs_gpio.pin));

// 	if (drdy_gpio.port != GPIO::PortInvalid) {
// 		ret.drdy_gpio = stm32_get_pinnode(getGPIOPort(drdy_gpio.port), getGPIOPin(drdy_gpio.pin));
// 	}

// 	if (PX4_SPIDEVID_TYPE(devid) == 0) { // it's a PX4 device (internal or external)
// 		ret.devid = PX4_SPIDEV_ID(PX4_SPI_DEVICE_ID, devid);

// 	} else { // it's a NuttX device (e.g. SPIDEV_FLASH(0))
// 		ret.devid = devid;
// 	}

// 	ret.devtype_driver = PX4_SPI_DEV_ID(devid);
// 	return ret;
// }

// static inline constexpr px4_spi_bus_t initSPIBus(
//     SPI::Bus bus,
//     const px4_spi_bus_devices_t &devices,
// 	GPIO::GPIOPin power_enable = {})
// {
// 	px4_spi_bus_t ret{};
// 	ret.requires_locking = false;

// 	for (int i = 0; i < SPI_BUS_MAX_DEVICES; ++i) {
// 		ret.devices[i] = devices.devices[i];

// 		// check that the same device is configured only once (the chip-select code depends on that)
// 		// for (int j = i + 1; j < SPI_BUS_MAX_DEVICES; ++j) {
// 		// 	if (ret.devices[j].cs_gpio != 0) {
// 		// 		constexpr_assert(ret.devices[i].devid != ret.devices[j].devid, "Same device configured multiple times");
// 		// 	}
// 		// }

// 		if (ret.devices[i].cs_gpio != 0) {
// 			// A bus potentially requires locking if it is accessed by non-PX4 devices (i.e. NuttX drivers)
// 			if (PX4_SPI_DEVICE_ID != PX4_SPIDEVID_TYPE(ret.devices[i].devid)) {
// 				ret.requires_locking = true;
// 			}
// 		}
// 	}

// 	ret.bus = (int)bus;
// 	ret.is_external = false;

// 	if (power_enable.port != GPIO::PortInvalid) {
//         ret.power_enable_gpio = stm32_get_pinnode(getGPIOPort(power_enable.port), getGPIOPin(power_enable.pin));
// 	}

// 	return ret;
// }

