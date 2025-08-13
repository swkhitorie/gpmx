#include <device/serial.h>

#define SERIAL_DEVNAME_FMT    "/dev/ttyS%d"
#define SERIAL_DEVNAME_FMTLEN (9 + 3 + 1)

int serial_register(struct uart_dev_s *dev, int bus)
{
    int ret = 0;
    char devname[SERIAL_DEVNAME_FMTLEN];
    snprintf(devname, SERIAL_DEVNAME_FMTLEN, SERIAL_DEVNAME_FMT, bus);

    ret = dn_register(devname, dev);

    return ret;
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

    return xSemaphoreTake(dev->exclsem, 1000);
#else

    if (dev->flag_excl == 0x01) {
        dev->flag_excl = 0x00;
        return DTRUE;
    } else {
        return DFALSE;
    }
#endif
}

int serial_dev_unlock(struct uart_dev_s *dev)
{
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    return xSemaphoreGive(dev->exclsem);
#else

    dev->flag_excl = 0x01;
    return DTRUE;
#endif
}

int serial_tx_wait(struct uart_dev_s *dev)
{
	int ret = DTRUE;
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    return xSemaphoreTake(dev->xmitsem, 0);
#else

    if (dev->flag_tx != 0x01) {
		return DFALSE;
	}
    dev->flag_tx = 0x00;
    return DTRUE;
#endif
}

void serial_tx_post(struct uart_dev_s *dev)
{
	int ret = DTRUE;
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

#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    vPortEnterCritical();
#else

    dn_disable_irq();
#endif

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

#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    vPortExitCritical();
#else

    dn_enable_irq();
#endif

	return wlen;
}

uint16_t serial_buf_read(struct uart_buffer_s *obj, uint8_t *p, uint16_t len)
{
	uint16_t i;
	uint16_t rlen = 0;

#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    vPortEnterCritical();
#else

    dn_disable_irq();
#endif

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

#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    vPortExitCritical();
#else

    dn_enable_irq();
#endif

	return rlen;
}

void serial_buf_clear(struct uart_buffer_s *obj)
{
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    vPortEnterCritical();
#else

    dn_disable_irq();
#endif

	for (int i = 0; i < obj->capacity; i++) {
		obj->buffer[i] = 0;
	}
	obj->size = 0;
	obj->in = 0;
	obj->out = 0;

#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    vPortExitCritical();
#else

    dn_enable_irq();
#endif
}

