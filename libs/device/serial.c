#include <device/serial.h>

#define SERIAL_DEVNAME_FMT    "/dev/ttyS%d"
#define SERIAL_DEVNAME_FMTLEN (9 + 3 + 1)

int serial_register(struct uart_dev_s *dev, int bus)
{
    int ret = 0;
    char devname[SERIAL_DEVNAME_FMTLEN];
    snprintf(devname, SERIAL_DEVNAME_FMTLEN, SERIAL_DEVNAME_FMT, bus);

    ret = dregister(devname, dev);

    return ret;
}

struct uart_dev_s* serial_bus_get(int bus)
{
    struct uart_dev_s *dev;
    char devname[SERIAL_DEVNAME_FMTLEN];
    snprintf(devname, SERIAL_DEVNAME_FMTLEN, SERIAL_DEVNAME_FMT, bus);

    dev = dbind(devname);

    return dev;
}

int serial_bus_initialize(int bus)
{
    struct uart_dev_s *dev = serial_bus_get(bus);

    if (!dev) {
        return -1;
    }

    return dev->ops->setup(dev);
}

uint16_t uart_buf_write(struct uart_buffer_s *obj, const uint8_t *p, uint16_t len)
{
	uint16_t i;
	uint16_t wlen = 0;
	uint16_t rssize = 0;

	rssize = obj->capacity - obj->size;
	if (rssize >= len) {
		wlen = len;
	} else {
		wlen = rssize;
	}

	for (i = 0; i < wlen; i++) {
		obj->buffer[obj->in] = p[i];
		obj->in++;
		if (obj->in >= obj->capacity) {
			obj->in -= obj->capacity;
		}
        obj->size++;
	}

	return wlen;
}

uint16_t uart_buf_read(struct uart_buffer_s *obj, uint8_t *p, uint16_t len)
{
	uint16_t i;
	uint16_t rlen = 0;

	if (obj->size >= len) {
		rlen = len;
	} else {
		rlen = obj->size;
	}

	for (i = 0; i < rlen; i++) {
		p[i] = obj->buffer[obj->out];
		obj->out++;
		if (obj->out >= obj->capacity) {
			obj->out -= obj->capacity;
		}
        obj->size--;
	}

	return rlen;
}

void uart_buf_clear(struct uart_buffer_s *obj)
{
	for (int i = 0; i < obj->capacity; i++) {
		obj->buffer[i] = 0;
	}
	obj->size = 0;
	obj->in = 0;
	obj->out = 0;
}

int uart_register(const char *path, uart_dev_t *dev)
{

	return 0;
}
