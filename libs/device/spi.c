#include <device/spi.h>

int spi_register(const char *path, struct spi_dev_s *dev)
{

    return 0;
}

#if defined(CONFIG_BOARD_FREERTOS_ENABLE) && defined(CONFIG_SPI_TASKSYNC)

void spi_sem_init(struct spi_dev_s *dev)
{
    dev->exclsem = xSemaphoreCreateBinary();

    /* This semaphore is used for signaling and, hence, should not have
    * priority inheritance enabled.
    */
    dev->rxsem = xSemaphoreCreateBinary();
    dev->txsem = xSemaphoreCreateBinary();
    dev->rtxsem = xSemaphoreCreateBinary();
    xSemaphoreTake(dev->rxsem, 1);
    xSemaphoreTake(dev->txsem, 1);
    xSemaphoreTake(dev->rtxsem, 1);
}

void spi_sem_destroy(struct spi_dev_s *dev)
{
    vSemaphoreDelete(dev->rxsem);
    vSemaphoreDelete(dev->txsem);
    vSemaphoreDelete(dev->rtxsem);
    vSemaphoreDelete(dev->exclsem);
}

int spi_dmarxwait(struct spi_dev_s *dev, bool rxresult)
{
    int ret;

    /* Take the semaphore (perhaps waiting).  If the result is zero, then the
     * DMA must not really have completed???
     */

    do {
        ret = xSemaphoreTake(dev->rxsem, portMAX_DELAY);
    }while (!rxresult && ret == pdFALSE);

    return ret;
}

void spi_dmarxwakeup(struct spi_dev_s *dev)
{
    xSemaphoreGive(dev->rxsem);
}

int spi_dmatxwait(struct spi_dev_s *dev, bool txresult)
{
    int ret;

    /* Take the semaphore (perhaps waiting).  If the result is zero, then the
     * DMA must not really have completed???
     */

    do {
        ret = xSemaphoreTake(dev->txsem, portMAX_DELAY);
    }while (!txresult && ret == pdFALSE);

    return ret;
}

void spi_dmatxwakeup(struct spi_dev_s *dev)
{
    xSemaphoreGive(dev->txsem);
}

int spi_dmartxwait(struct spi_dev_s *dev, bool rtxresult)
{
    int ret;

    /* Take the semaphore (perhaps waiting).  If the result is zero, then the
     * DMA must not really have completed???
     */

    do {
        ret = xSemaphoreTake(dev->rtxsem, portMAX_DELAY);
    }while (!rtxresult && ret == pdFALSE);

    return ret;
}

void spi_dmartxwakeup(struct spi_dev_s *dev)
{
    xSemaphoreGive(dev->rtxsem);
}

#endif