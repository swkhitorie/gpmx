#include <gpm/spi/qspi.h>
#include <stdio.h>

#ifdef CONFIG_GPDRIVE_QUADSPI

#define QUADSPI_DEVNAME_FMT    "/dev/qspi%d"
#define QUADSPI_DEVNAME_FMTLEN (9 + 3 + 1)

int qspi_register(struct qspi_dev_s *spi, int bus)
{
    char devname[QUADSPI_DEVNAME_FMTLEN];
    snprintf(devname, QUADSPI_DEVNAME_FMTLEN, QUADSPI_DEVNAME_FMT, bus);

    if (!up_register(devname, spi)) {
        return -1;
    }

    return GOK;
}

struct qspi_dev_s *qspi_bus_get(int bus)
{
    struct qspi_dev_s *dev;
    char devname[QUADSPI_DEVNAME_FMTLEN];
    snprintf(devname, QUADSPI_DEVNAME_FMTLEN, QUADSPI_DEVNAME_FMT, bus);

    dev = up_bind(devname);

    return dev;
}

int qspi_bus_initialize(int bus)
{
    struct qspi_dev_s *dev = qspi_bus_get(bus);

    if (!dev) {
        return -1;
    }

#if defined(CONFIG_FREERTOS_ENABLE)

    dev->exclsem = xSemaphoreCreateBinary();

    /* This semaphore is used for signaling and, hence, should not have
    * priority inheritance enabled.
    */
    dev->rxsem = xSemaphoreCreateBinary();
    dev->txsem = xSemaphoreCreateBinary();

    xSemaphoreGive(dev->exclsem);
#elif defined(CONFIG_RTTNANO_ENABLE)

    char *excl_name = "qspi0_excl";
    char *tx_name = "qspi0_tx";
    char *rx_name = "qspi0_rx";
    excl_name[4] = '0' + (bus - 0);
    tx_name[4] = '0' + (bus - 0);
    rx_name[4] = '0' + (bus - 0);

    dev->exclsem = rt_sem_create(excl_name, 1, RT_IPC_FLAG_PRIO);
    dev->rxsem = rt_sem_create(rx_name, 0, RT_IPC_FLAG_PRIO);
    dev->txsem = rt_sem_create(tx_name, 0, RT_IPC_FLAG_PRIO);
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

#if defined(CONFIG_FREERTOS_ENABLE)

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
#elif defined(CONFIG_RTTNANO_ENABLE)

    if (lock) {
        if (RT_EOK == rt_sem_take(dev->exclsem, 10)) {
            return GOK;
        } else {
            return -1;
        }
	} else {
        rt_sem_release(dev->exclsem);
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
#if defined(CONFIG_FREERTOS_ENABLE)

    int ret = GOK;
    /* Take the semaphore (perhaps waiting).  If the result is zero, then the
     * DMA must not really have completed???
     */

    do {
        ret = xSemaphoreTake(dev->txsem, 20);
    }while (ret == pdFALSE);

    return GOK;
#elif defined(CONFIG_RTTNANO_ENABLE)

    int ret = GOK;
    do {
        ret = rt_sem_take(dev->txsem, 20);
    }while (ret != RT_EOK);

    return GOK;
#else

    uint32_t timeout_ms = 20;
    uint32_t time_start = up_time();
    do {
        if (dev->flag_tx == 0x01) {
            dev->flag_tx = 0x00;
            return GOK;
        }
    } while ((up_time() - time_start) <= timeout_ms);

    if ((up_time() - time_start) > timeout_ms) {
        return -1;
    }

    return GOK;
#endif
}

void qspi_txwakeup(struct qspi_dev_s *dev)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    xSemaphoreGive(dev->txsem);
#elif defined(CONFIG_RTTNANO_ENABLE)

    rt_sem_release(dev->txsem);
#else

    dev->flag_tx = 0x01;
#endif
}

int qspi_rxwait(struct qspi_dev_s *dev)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    int ret = GOK;
    /* Take the semaphore (perhaps waiting).  If the result is zero, then the
     * DMA must not really have completed???
     */

    do {
        ret = xSemaphoreTake(dev->rxsem, 20);
    }while (ret == pdFALSE);

    return GOK;
#elif defined(CONFIG_RTTNANO_ENABLE)

    int ret = GOK;
    do {
        ret = rt_sem_take(dev->rxsem, 20);
    }while (ret != RT_EOK);

    return GOK;
#else

    uint32_t timeout_ms = 20;
    uint32_t time_start = up_time();
    do {
        if (dev->flag_rx == 0x01) {
            dev->flag_rx = 0x00;
            return GOK;
        }
    } while ((up_time() - time_start) <= timeout_ms);

    if ((up_time() - time_start) > timeout_ms) {
        return -1;
    }

    return GOK;
#endif
}

void qspi_rxwakeup(struct qspi_dev_s *dev)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    xSemaphoreGive(dev->rxsem);
#elif defined(CONFIG_RTTNANO_ENABLE)

    rt_sem_release(dev->rxsem);
#else

    dev->flag_rx = 0x01;
#endif
}

#endif // end with macro CONFIG_GPDRIVE_QUADSPI
