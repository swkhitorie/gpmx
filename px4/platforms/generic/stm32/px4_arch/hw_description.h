#pragma once

#include <stdint.h>
#include <drv_gpio.h>

namespace GPIO
{

enum Port {
	PortInvalid = 0,
	PortA,
	PortB,
	PortC,
	PortD,
	PortE,
	PortF,
	PortG,
	PortH,
	PortI,
	PortJ,
	PortK,
};

enum Pin {
	Pin0 = 0,
	Pin1,
	Pin2,
	Pin3,
	Pin4,
	Pin5,
	Pin6,
	Pin7,
	Pin8,
	Pin9,
	Pin10,
	Pin11,
	Pin12,
	Pin13,
	Pin14,
	Pin15,
};

struct GPIOPin {
	Port port;
	Pin pin;
};

}

static inline constexpr uint32_t getGPIOPort(GPIO::Port port)
{
	switch (port) {
	case GPIO::PortA: return GPIOA_BASE;

	case GPIO::PortB: return GPIOB_BASE;

	case GPIO::PortC: return GPIOC_BASE;

#if defined(GPIOC)
	case GPIO::PortD: return GPIOD_BASE;
#endif

#if defined(GPIOE)
	case GPIO::PortE: return GPIOE_BASE;
#endif

#if defined(GPIOF)
	case GPIO::PortF: return GPIOF_BASE;
#endif

#if defined(GPIOG)
	case GPIO::PortG: return GPIOG_BASE;
#endif

#if defined(GPIOH)
	case GPIO::PortH: return GPIOH_BASE;
#endif

#if defined(GPIOI)
	case GPIO::PortI: return GPIOI_BASE;
#endif

#if defined(GPIOJ)
	case GPIO::PortJ: return GPIOJ_BASE;
#endif

#if defined(GPIOK)
	case GPIO::PortK: return GPIOK_BASE;
#endif

	default: break;
	}

	return 0;
}

static inline constexpr uint32_t getGPIOPin(GPIO::Pin pin)
{
	switch (pin) {
	case GPIO::Pin0: return 0;

	case GPIO::Pin1: return 1;

	case GPIO::Pin2: return 2;

	case GPIO::Pin3: return 3;

	case GPIO::Pin4: return 4;

	case GPIO::Pin5: return 5;

	case GPIO::Pin6: return 6;

	case GPIO::Pin7: return 7;

	case GPIO::Pin8: return 8;

	case GPIO::Pin9: return 9;

	case GPIO::Pin10: return 10;

	case GPIO::Pin11: return 11;

	case GPIO::Pin12: return 12;

	case GPIO::Pin13: return 13;

	case GPIO::Pin14: return 14;

	case GPIO::Pin15: return 15;
	}

	return 0;
}

namespace SPI
{

enum class Bus {
	SPIBUS1 = 1,
	SPIBUS2,
	SPIBUS3,
	SPIBUS4,
	SPIBUS5,
	SPIBUS6,
};

using CS = GPIO::GPIOPin; ///< chip-select pin
using DRDY = GPIO::GPIOPin; ///< data ready pin

struct bus_device_external_cfg_t {
	CS cs_gpio;
	DRDY drdy_gpio;
};

} // namespace SPI

static inline constexpr uint32_t stm32_get_pinnode(uint32_t port_base, uint8_t pin) {
    return (uint32_t)((16 * ( ((uint32_t)(port_base) - (uint32_t)GPIOA_BASE)/(0x0400UL) )) + pin);
}
