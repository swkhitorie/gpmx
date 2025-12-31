#include <device/serial.h>

#ifdef CONFIG_GPDRIVE_SERIAL

#define SERIAL_DEVNAME_FMT    "/dev/ttyS%d"
#define SERIAL_DEVNAME_FMTLEN (9 + 3 + 1)

int serial_register(struct uart_dev_s *dev, int bus)
{
    char devname[SERIAL_DEVNAME_FMTLEN];
    snprintf(devname, SERIAL_DEVNAME_FMTLEN, SERIAL_DEVNAME_FMT, bus);

    if (!dn_register(devname, dev)) {
        return -1;
    }

    return GOK;
}

struct uart_dev_s* serial_bus_get(int bus)
{
    struct uart_dev_s *dev;
    char devname[SERIAL_DEVNAME_FMTLEN];
    snprintf(devname, SERIAL_DEVNAME_FMTLEN, SERIAL_DEVNAME_FMT, bus);

    dev = dn_bind(devname);

    return dev;
}

int serial_bus_initialize(int bus)
{
    struct uart_dev_s *dev = serial_bus_get(bus);

    if (!dev) {
        return -1;
    }

#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    dev->exclsem = xSemaphoreCreateBinary();

    /* This semaphore is used for signaling and, hence, should not have
    * priority inheritance enabled.
    */
    dev->xmitsem = xSemaphoreCreateBinary();

    xSemaphoreGive(dev->exclsem);
    xSemaphoreGive(dev->xmitsem);
#else

    dev->flag_excl = 0x01;
	dev->flag_tx = 0x01;
#endif

    return dev->ops->setup(dev);
}

int serial_dev_lock(struct uart_dev_s *dev)
{
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    if (pdTRUE == xSemaphoreTake(dev->exclsem, 0)) {
        return GOK;
    } else {
        return -1;
    }
#else

    if (dev->flag_excl == 0x01) {
        dev->flag_excl = 0x00;
        return GOK;
    } else {
        return -1;
    }
#endif
}

int serial_dev_unlock(struct uart_dev_s *dev)
{
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    xSemaphoreGive(dev->exclsem);
    return GOK;
#else

    dev->flag_excl = 0x01;
    return GOK;
#endif
}

int serial_tx_wait(struct uart_dev_s *dev)
{
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    if (pdTRUE == xSemaphoreTake(dev->xmitsem, 5)) {
        return -1;
    } else {
        return GOK;
    }
#else

    if (dev->flag_tx != 0x01) {
		return -1;
	}
    dev->flag_tx = 0x00;
    return GOK;
#endif
}

void serial_tx_post(struct uart_dev_s *dev)
{
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    BaseType_t h_pri = pdFALSE;
    xSemaphoreGiveFromISR(dev->xmitsem, &h_pri);
    portYIELD_FROM_ISR(h_pri);
#else

    dev->flag_tx = 0x01;
#endif
}

uint16_t serial_buf_write(struct uart_buffer_s *obj, const uint8_t *p, uint16_t len)
{
	uint16_t i;
	uint16_t wlen = 0;
	uint16_t rssize = 0;

    gpdrv_irq_disable();

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

    gpdrv_irq_enable();

	return wlen;
}

uint16_t serial_buf_read(struct uart_buffer_s *obj, uint8_t *p, uint16_t len)
{
	uint16_t i;
	uint16_t rlen = 0;

    gpdrv_irq_disable();

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

    gpdrv_irq_enable();

	return rlen;
}

void serial_buf_clear(struct uart_buffer_s *obj)
{
    gpdrv_irq_disable();

	for (int i = 0; i < obj->capacity; i++) {
		obj->buffer[i] = 0;
	}
	obj->size = 0;
	obj->in = 0;
	obj->out = 0;

    gpdrv_irq_enable();
}

#endif  // end with macro CONFIG_GPDRIVE_SERIAL
