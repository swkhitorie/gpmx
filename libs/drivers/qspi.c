#include <device/qspi.h>

#ifdef CONFIG_GPDRIVE_QUADSPI

#define QUADSPI_DEVNAME_FMT    "/dev/qspi%d"
#define QUADSPI_DEVNAME_FMTLEN (9 + 3 + 1)

int qspi_register(struct qspi_dev_s *spi, int bus)
{
    char devname[QUADSPI_DEVNAME_FMTLEN];
    snprintf(devname, QUADSPI_DEVNAME_FMTLEN, QUADSPI_DEVNAME_FMT, bus);

    if (!dn_register(devname, spi)) {
        return -1;
    }

    return GOK;
}

struct qspi_dev_s *qspi_bus_get(int bus)
{
    struct qspi_dev_s *dev;
    char devname[QUADSPI_DEVNAME_FMTLEN];
    snprintf(devname, QUADSPI_DEVNAME_FMTLEN, QUADSPI_DEVNAME_FMT, bus);

    dev = dn_bind(devname);

    return dev;
}

int qspi_bus_initialize(int bus)
{
    struct qspi_dev_s *dev = qspi_bus_get(bus);

    if (!dev) {
        return -1;
    }

#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    dev->exclsem = xSemaphoreCreateBinary();

    /* This semaphore is used for signaling and, hence, should not have
    * priority inheritance enabled.
    */
    dev->rxsem = xSemaphoreCreateBinary();
    dev->txsem = xSemaphoreCreateBinary();

    xSemaphoreGive(dev->exclsem);
#else

    dev->flag_tx = 0;
    dev->flag_rx = 0;

    dev->flag_excl = 0x01;
#endif

    return dev->ops->setup(dev);
}

int  qspi_devlock(struct qspi_dev_s *dev, bool lock)
{
    int ret = GOK;

#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    if (lock) {
        if (pdTRUE == xSemaphoreTake(dev->exclsem, 10)) {
            return GOK;
        } else {
            return -1;
        }
	} else {
        xSemaphoreGive(dev->exclsem);
        return GOK;
    }
#else

    if (lock) {
        if (dev->flag_excl == 0x01) {
            dev->flag_excl = 0x00;
            return GOK;
        } else {
            return -1;
        }
	} else {
        dev->flag_excl = 0x01;
        return GOK;
    }
#endif
}

int qspi_txwait(struct qspi_dev_s *dev)
{
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    int ret = GOK;
    /* Take the semaphore (perhaps waiting).  If the result is zero, then the
     * DMA must not really have completed???
     */

    do {
        ret = xSemaphoreTake(dev->txsem, 20);
    }while (ret == pdFALSE);

    return GOK;
#else

    uint32_t timeout_ms = 20;
    uint32_t time_start = dn_time();
    do {
        if (dev->flag_tx == 0x01) {
            dev->flag_tx = 0x00;
            return GOK;
        }
    } while ((dn_time() - time_start) <= timeout_ms);

    if ((dn_time() - time_start) > timeout_ms) {
        return -1;
    }
#endif
}

void qspi_txwakeup(struct qspi_dev_s *dev)
{
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    xSemaphoreGive(dev->txsem);
#else

    dev->flag_tx = 0x01;
#endif
}

int qspi_rxwait(struct qspi_dev_s *dev)
{
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    int ret = GOK;
    /* Take the semaphore (perhaps waiting).  If the result is zero, then the
     * DMA must not really have completed???
     */

    do {
        ret = xSemaphoreTake(dev->rxsem, 20);
    }while (ret == pdFALSE);

    return GOK;
#else

    uint32_t timeout_ms = 20;
    uint32_t time_start = dn_time();
    do {
        if (dev->flag_rx == 0x01) {
            dev->flag_rx = 0x00;
            return GOK;
        }
    } while ((dn_time() - time_start) <= timeout_ms);

    if ((dn_time() - time_start) > timeout_ms) {
        return -1;
    }
#endif
}

void qspi_rxwakeup(struct qspi_dev_s *dev)
{
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    xSemaphoreGive(dev->rxsem);
#else

    dev->flag_rx = 0x01;
#endif
}

#endif // end with macro CONFIG_GPDRIVE_QUADSPI
