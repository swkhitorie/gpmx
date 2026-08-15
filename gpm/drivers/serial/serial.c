#include <gpm/serial/serial.h>
#include <gpmx/config.h>

#include <stdio.h>

#ifdef CONFIG_DRIVER_SERIAL

#define SERIAL_DEVNAME_FMT    "/dev/ttyS%d"
#define SERIAL_DEVNAME_FMTLEN (9 + 3 + 1)

int serial_register(struct uart_dev_s *dev, int bus)
{
    char devname[SERIAL_DEVNAME_FMTLEN];
    snprintf(devname, SERIAL_DEVNAME_FMTLEN, SERIAL_DEVNAME_FMT, bus);

    if (!up_register(devname, dev)) {
        return -1;
    }

    return GOK;
}

struct uart_dev_s* serial_bus_get(int bus)
{
    struct uart_dev_s *dev;
    char devname[SERIAL_DEVNAME_FMTLEN];
    snprintf(devname, SERIAL_DEVNAME_FMTLEN, SERIAL_DEVNAME_FMT, bus);

    dev = up_bind(devname);

    return dev;
}

int serial_bus_initialize(int bus)
{
    struct uart_dev_s *dev = serial_bus_get(bus);

    if (!dev) {
        return -1;
    }

#if defined(CONFIG_FREERTOS_ENABLE)

    dev->exclsem = xSemaphoreCreateBinary();

    /* This semaphore is used for signaling and, hence, should not have
    * priority inheritance enabled.
    */
    dev->xmitsem = xSemaphoreCreateBinary();

    xSemaphoreGive(dev->exclsem);
    xSemaphoreGive(dev->xmitsem);
#elif defined(CONFIG_RTTNANO_ENABLE)

    char *excl_name = "ttyS0_excl";
    char *xmit_name = "ttyS0_xmit";
    excl_name[4] = '0' + (bus - 0);
    xmit_name[4] = '0' + (bus - 0);

    dev->exclsem = rt_sem_create(excl_name, 1, RT_IPC_FLAG_PRIO);
    dev->xmitsem = rt_sem_create(xmit_name, 1, RT_IPC_FLAG_PRIO);
#else

    dev->flag_excl = 0x01;
	dev->flag_tx = 0x01;
#endif

    return dev->ops->setup(dev);
}

int serial_dev_lock(struct uart_dev_s *dev)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    if (pdTRUE == xSemaphoreTake(dev->exclsem, 0)) {
        return GOK;
    } else {
        return -1;
    }
#elif defined(CONFIG_RTTNANO_ENABLE)

    if (RT_EOK == rt_sem_take(dev->exclsem, 0)) {
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
#if defined(CONFIG_FREERTOS_ENABLE)

    xSemaphoreGive(dev->exclsem);
    return GOK;
#elif defined(CONFIG_RTTNANO_ENABLE)

    rt_sem_release(dev->exclsem);
    return GOK;
#else

    dev->flag_excl = 0x01;
    return GOK;
#endif
}

int serial_tx_wait(struct uart_dev_s *dev)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    if (pdTRUE == xSemaphoreTake(dev->xmitsem, 5)) {
        return GOK;
    } else {
        return -1;
    }
#elif defined(CONFIG_RTTNANO_ENABLE)

    if (RT_EOK == rt_sem_take(dev->xmitsem, 5)) {
        return GOK;
    } else {
        return -1;
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
#if defined(CONFIG_FREERTOS_ENABLE)

    BaseType_t h_pri = pdFALSE;
    xSemaphoreGiveFromISR(dev->xmitsem, &h_pri);
    portYIELD_FROM_ISR(h_pri);
#elif defined(CONFIG_RTTNANO_ENABLE)

    rt_sem_release(dev->xmitsem);
#else

    dev->flag_tx = 0x01;
#endif
}

uint16_t serial_buf_write(struct uart_buffer_s *obj, const uint8_t *p, uint16_t len)
{
	uint16_t i;
	uint16_t wlen = 0;
	uint16_t rssize = 0;

    // enter_critical_section();

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

    // leave_critical_section(0);

	return wlen;
}

uint16_t serial_buf_read(struct uart_buffer_s *obj, uint8_t *p, uint16_t len)
{
	uint16_t i;
	uint16_t rlen = 0;

    // enter_critical_section();

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

    // leave_critical_section(0);

	return rlen;
}

void serial_buf_clear(struct uart_buffer_s *obj)
{
    // enter_critical_section();

	for (int i = 0; i < obj->capacity; i++) {
		obj->buffer[i] = 0;
	}
	obj->size = 0;
	obj->in = 0;
	obj->out = 0;

    // leave_critical_section(0);
}

uint16_t serial_buf_size(struct uart_buffer_s *obj)
{
    return obj->size;
}

#if defined(CONFIG_LIBC_VFS)

/* Character driver methods */
#include <poll.h>
static int     uart_open(struct file *filep);
static int     uart_close(struct file *filep);
static ssize_t uart_read(struct file *filep,
                         char *buffer, size_t buflen);
static ssize_t uart_write(struct file *filep,
                        const char *buffer,
                        size_t buflen);
static int     uart_ioctl(struct file *filep,
                        int cmd, unsigned long arg);
static int     uart_poll(struct file *filep,
                         struct pollfd *fds, bool setup);

static const struct file_operations g_serialops =
{
    uart_open,  /* open */
    uart_close, /* close */
    uart_read,  /* read */
    uart_write, /* write */
    NULL,       /* seek */
    uart_ioctl, /* ioctl */
    uart_poll   /* poll */
};

int uart_register(const char *path, uart_dev_t *dev)
{
    sem_init(&dev->pollsem, 0, 1);
    return register_driver(path, &g_serialops, 0666, dev);
}

static int uart_takesem(sem_t *sem, bool errout)
{
    if (errout) {
        return sem_wait(sem);
    } else {
        return sem_wait_uninterruptible(sem);
    }
}

#define uart_givesem(sem) sem_post(sem)

static void uart_pollnotify(uart_dev_t *dev, pollevent_t eventset)
{
    int i;

    for (i = 0; i < CONFIG_SERIAL_NPOLLWAITERS; i++)
    {
        struct pollfd *fds = dev->fds[i];

        if (fds) {

            fds->revents |= (fds->events & eventset);

            if (fds->revents != 0) {

                int semcount;

                sem_getvalue(fds->sem, &semcount);
                if (semcount < 1) {
                    sem_post(fds->sem);
                }
            }
        }
    }
}

static int uart_open(struct file *filep)
{
    return 0;
}

static int uart_close(struct file *filep)
{
    return 0;
}

static ssize_t uart_read(struct file *filep,
                         char *buffer, size_t buflen)
{
    struct inode *inode = filep->f_inode;
    uart_dev_t   *dev   = inode->i_private;

    size_t rsz = SERIAL_RDBUF(dev, buffer, buflen);

    return rsz;
}

static ssize_t uart_write(struct file *filep,
                        const char *buffer,
                        size_t buflen)
{
    struct inode *inode = filep->f_inode;
    uart_dev_t   *dev   = inode->i_private;

    size_t rsz = SERIAL_SEND(dev, buffer, buflen);

    return rsz;
}

static int uart_ioctl(struct file *filep,
                        int cmd, unsigned long arg)
{
    return 0;
}

static int uart_poll(struct file *filep,
                         struct pollfd *fds, bool setup)
{
    struct inode *inode = filep->f_inode;
    uart_dev_t   *dev   = inode->i_private;
    pollevent_t   eventset;
    int           ndx;
    int           ret;
    int           i;

    /* Are we setting up the poll?  Or tearing it down? */
    ret = uart_takesem(&dev->pollsem, true);

    if (ret < 0) {
        return ret;
    }

    if (setup) {

        /* This is a request to set up the poll.  Find an available
        * slot for the poll structure reference
        */

        for (i = 0; i < CONFIG_SERIAL_NPOLLWAITERS; i++) {
            /* Find an available slot */
            if (!dev->fds[i]) {
                /* Bind the poll structure and this slot */
                dev->fds[i]  = fds;
                fds->priv    = &dev->fds[i];
                break;
            }
        }

        if (i >= CONFIG_SERIAL_NPOLLWAITERS) {
            fds->priv = NULL;
            ret       = -EBUSY;
            goto errout;
        }

        /* Should we immediately notify on any of the requested events?
        * First, check if the xmit buffer is full.
        *
        * Get exclusive access to the xmit buffer indices.
        * NOTE: that we do not let this wait be interrupted by a signal
        * (we probably should, but that would be a little awkward).
        */

        eventset = 0;

        /* Check if the receive buffer is empty.
        *
        * Get exclusive access to the recv buffer indices.
        * NOTE: that we do not let this wait be interrupted by a signal
        * (we probably should, but that would be a little awkward).
        */

        if (SERIAL_RX_AVAILABLE(dev)) {
            eventset |= (fds->events & POLLIN);
        }

        if (eventset) {
            uart_pollnotify(dev, eventset);
        }
    }
    else if (fds->priv != NULL) {
        /* This is a request to tear down the poll. */
        struct pollfd **slot = (struct pollfd **)fds->priv;

        /* Remove all memory of the poll setup */
        *slot     = NULL;
        fds->priv = NULL;
    }

errout:
    uart_givesem(&dev->pollsem);
    return ret;
}
#endif

#endif  // end with macro CONFIG_DRIVER_SERIAL
