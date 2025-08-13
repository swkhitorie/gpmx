#include <device/spi.h>

#define SPI_DEVNAME_FMT    "/dev/spi%d"
#define SPI_DEVNAME_FMTLEN (8 + 3 + 1)

int spi_register(struct spi_dev_s *spi, int bus)
{
    int ret = 0;
    char devname[SPI_DEVNAME_FMTLEN];
    snprintf(devname, SPI_DEVNAME_FMTLEN, SPI_DEVNAME_FMT, bus);

    ret = dn_register(devname, spi);

    return ret;
}

struct spi_dev_s *spi_bus_get(int bus)
{
    struct spi_dev_s *dev;
    char devname[SPI_DEVNAME_FMTLEN];
    snprintf(devname, SPI_DEVNAME_FMTLEN, SPI_DEVNAME_FMT, bus);

    dev = dn_bind(devname);

    return dev;
}

int spi_bus_initialize(int bus)
{
    struct spi_dev_s *dev = spi_bus_get(bus);

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

int spi_devlock(struct spi_dev_s *dev, bool lock)
{
    int ret = DTRUE;

#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    if (lock) {
        ret = xSemaphoreTake(dev->exclsem, 1000);
	} else {
        ret = xSemaphoreGive(dev->exclsem);
    }
#else

    if (lock) {
        if (dev->flag_excl == 0x01) {
            dev->flag_excl = 0x00;
            ret = DTRUE;
        } else {
            ret = DFALSE;
        }
	} else {
        dev->flag_excl = 0x01;
    }
#endif

    return ret;
}

int spi_dmarxwait(struct spi_dev_s *dev)
{
    int ret = DTRUE;

#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    /* Take the semaphore (perhaps waiting).  If the result is zero, then the
     * DMA must not really have completed???
     */

    do {
        ret = xSemaphoreTake(dev->rxsem, portMAX_DELAY);
    }while (dev->rxresult == 0 && ret == pdFALSE);
#else

    uint32_t timeout_ms = 20;
    uint32_t time_start = dn_timems();
    do {
        if (dev->flag_rx == 0x01) {
            dev->flag_rx = 0x00;
            ret = DTRUE;
            break;
        }
    } while ((dn_timems() - time_start) <= timeout_ms);

    if ((dn_timems() - time_start) > timeout_ms) {
        ret = DFALSE;
    }
#endif

    return ret;
}

int spi_dmatxwait(struct spi_dev_s *dev)
{
    int ret = DTRUE;

#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    /* Take the semaphore (perhaps waiting).  If the result is zero, then the
     * DMA must not really have completed???
     */

    do {
        ret = xSemaphoreTake(dev->txsem, portMAX_DELAY);
    }while (dev->txresult == 0 && ret == pdFALSE);
#else

    uint32_t timeout_ms = 20;
    uint32_t time_start = dn_timems();
    do {
        if (dev->flag_tx == 0x01) {
            dev->flag_tx = 0x00;
            ret = DTRUE;
            break;
        }
    } while ((dn_timems() - time_start) <= timeout_ms);

    if ((dn_timems() - time_start) > timeout_ms) {
        ret = DFALSE;
    }
#endif

    return ret;
}

void spi_dmarxwakeup(struct spi_dev_s *dev)
{
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    xSemaphoreGive(dev->rxsem);
#else

    dev->flag_rx = 0x01;
#endif
}

void spi_dmatxwakeup(struct spi_dev_s *dev)
{
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    xSemaphoreGive(dev->txsem);
#else

    dev->flag_tx = 0x01;
#endif
}

