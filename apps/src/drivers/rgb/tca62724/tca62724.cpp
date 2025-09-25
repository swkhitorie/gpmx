#include "tca62724.h"
#include <device/i2c_master.h>

#define ADDR			    0x55	/**< I2C adress of TCA62724FMG */
#define SUB_ADDR_START		0x01	/**< write everything (with auto-increment) */
#define SUB_ADDR_PWM0		0x81	/**< blue     (without auto-increment) */
#define SUB_ADDR_PWM1		0x82	/**< green    (without auto-increment) */
#define SUB_ADDR_PWM2		0x83	/**< red      (without auto-increment) */
#define SUB_ADDR_SETTINGS	0x84	/**< settings (without auto-increment)*/

#define SETTING_NOT_POWERSAVE	0x01	/**< power-save mode not off */
#define SETTING_ENABLE   	    0x02	/**< on */

struct i2c_master_s *bus;
bool _leds_enabled = true;
float _brightness = 15.0f;
float _max_brightness = 1.0f;
uint8_t	 _r = 0;
uint8_t	 _g = 0;
uint8_t	 _b = 0;

static int i2c_register_write(uint8_t reg, uint8_t data)
{
    int ret;
    uint8_t cmd[2] {};
    cmd[0] = reg;
    cmd[1] = data;

    struct i2c_msg_s msg;
    msg.frequency = 100000;
    msg.addr = ADDR;
    msg.buffer = &cmd[0];
    msg.length = 2;
    msg.flags = 0;

    ret = I2C_TRANSFER(bus, &msg, 1);
    return ret;
}

static int i2c_register_read(uint8_t reg)
{
    uint8_t cmd[2] {};
    cmd[0] = reg;

    struct i2c_msg_s msg[2];
    msg[0].frequency = 100000;
    msg[0].addr = ADDR;
    msg[0].buffer = &cmd[0];
    msg[0].length = 1;
    msg[0].flags = 0;

    msg[1].frequency = 100000;
    msg[1].addr = ADDR;
    msg[1].buffer = &cmd[1];
    msg[1].length = 1;
    msg[1].flags = I2C_M_READ;

    int ret = I2C_TRANSFER(bus, msg, 2);

    return cmd[1];
}

static void i2c_register_reads(uint8_t reg, uint8_t *buf, uint16_t sz)
{
    int ret;
    uint8_t cmd;
    cmd = reg;

    struct i2c_msg_s msg[2];
    msg[0].frequency = 100000;
    msg[0].addr = ADDR;
    msg[0].buffer = &cmd;
    msg[0].length = 1;
    msg[0].flags = 0;

    msg[1].frequency = 100000;
    msg[1].addr = ADDR;
    msg[1].buffer = buf;
    msg[1].length = sz;
    msg[1].flags = I2C_M_READ;

    ret = I2C_TRANSFER(bus, msg, 2);
}

int send_led_enable(bool enable)
{
	if (_leds_enabled && enable) {
		// already enabled
		return 0;
	}

	_leds_enabled = enable;
	uint8_t settings_byte = 0;

	if (enable) {
		settings_byte |= SETTING_ENABLE;
	}

	settings_byte |= SETTING_NOT_POWERSAVE;

	const uint8_t msg[2] = { SUB_ADDR_SETTINGS, settings_byte};

	return i2c_register_write(SUB_ADDR_SETTINGS, settings_byte);
}

int send_led_rgb()
{
	/* To scale from 0..255 -> 0..15 shift right by 4 bits */
	uint8_t msgaa[6] = {
		SUB_ADDR_PWM0, (uint8_t)((_b >> 4) * _brightness * _max_brightness + 0.5f),
		SUB_ADDR_PWM1, (uint8_t)((_g >> 4) * _brightness * _max_brightness + 0.5f),
		SUB_ADDR_PWM2, (uint8_t)((_r >> 4) * _brightness * _max_brightness + 0.5f)
	};

    struct i2c_msg_s msg[2];
    msg[0].frequency = 100000;
    msg[0].addr = ADDR;
    msg[0].buffer = &msgaa[0];
    msg[0].length = 6;
    msg[0].flags = 0;

	return I2C_TRANSFER(bus, &msg[0], 1);
}

int get(bool *on, bool *powersave, uint8_t *r, uint8_t *g, uint8_t *b)
{
	uint8_t result[2] = {0, 0};
	int ret;

    struct i2c_msg_s msg[2];
    msg[0].frequency = 100000;
    msg[0].addr = ADDR;
    msg[0].buffer = &result[0];
    msg[0].length = 2;
    msg[0].flags = I2C_M_READ;

	ret = I2C_TRANSFER(bus, &msg[0], 1);

	if (ret == 0) {
		*on = ((result[0] >> 4) & SETTING_ENABLE);
		*powersave = !((result[0] >> 4) & SETTING_NOT_POWERSAVE);
		/* XXX check, looks wrong */
		*r = (result[0] & 0x0f) << 4;
		*g = (result[1] & 0xf0);
		*b = (result[1] & 0x0f) << 4;
	}

	return ret;
}


int rgb_probe()
{
	int ret;
	bool on, powersave;
	uint8_t r, g, b;

	/**
	   this may look strange, but is needed. There is a serial
	   EEPROM (Microchip-24aa01) that responds to a bunch of I2C
	   addresses, including the 0x55 used by this LED device. So
	   we need to do enough operations to be sure we are talking
	   to the right device. These 3 operations seem to be enough,
	   as the 3rd one consistently fails if no RGBLED is on the bus.
	 */

	if ((ret = get(&on, &powersave, &r, &g, &b)) != 0 ||
        (ret = send_led_enable(false) != 0) ||
        (ret = send_led_enable(false) != 0)) {
		return ret;
	}

	return ret;
}

int rgb_tca62724_init(int busn)
{
    int ret = 0;
    bus = i2c_bus_get(busn);

    if (!bus) {
        return -1;
    }

	/* switch off LED on start */
	send_led_enable(false);
	send_led_rgb();

    return 0;
}

int rgb_tca62724_set(uint8_t color, uint8_t brightness)
{
    int ret = 0;

    switch (color) {
    case RGB_COLOR_RED:
        _r = 255; _g = 0; _b = 0;
        send_led_enable(true);
        break;

    case RGB_COLOR_GREEN:
        _r = 0; _g = 255; _b = 0;
        send_led_enable(true);
        break;

    case RGB_COLOR_BLUE:
        _r = 0; _g = 0; _b = 255;
        send_led_enable(true);
        break;

    case RGB_COLOR_AMBER: //make it the same as yellow
    case RGB_COLOR_YELLOW:
        _r = 255; _g = 255; _b = 0;
        send_led_enable(true);
        break;

    case RGB_COLOR_PURPLE:
        _r = 255; _g = 0; _b = 255;
        send_led_enable(true);
        break;

    case RGB_COLOR_CYAN:
        _r = 0; _g = 255; _b = 255;
        send_led_enable(true);
        break;

    case RGB_COLOR_WHITE:
        _r = 255; _g = 255; _b = 255;
        send_led_enable(true);
        break;

    default:
        _r = 0; _g = 0; _b = 0;
        send_led_enable(false);
        break;
    }

    _brightness = (float)brightness / 255.f;

    send_led_rgb();

    return ret;
}
