#include <device/i2c_master.h>

#define I2C_DEVNAME_FMT    "/dev/i2c%d"
#define I2C_DEVNAME_FMTLEN (8 + 3 + 1)

/**
 * Synch Bus:
 * sem_excl: protect transfer process
 * sem_isr: i2c dev will get with time wait, until i2c isr give
 * 
 * example:
 * 
 * void i2c_transfer() {
 *     take sem_excl
 *     do {
 *        sem_timewait(&sem_isr)
 *     } (i2c transfer isr completed)
 *     give sem_excl
 * }
 * 
 * void i2c_transfer_isr_completed_callback() {
 *     sem_post(&sem_isr)
 * }
 */

int i2c_register(struct i2c_master_s *dev, int bus)
{
    int ret = 0;
    char devname[I2C_DEVNAME_FMTLEN];
    snprintf(devname, I2C_DEVNAME_FMTLEN, I2C_DEVNAME_FMT, bus);

    ret = dn_register(devname, dev);

    return ret;
}

struct i2c_master_s *i2c_bus_get(int bus)
{
    struct i2c_master_s *dev;
    char devname[I2C_DEVNAME_FMTLEN];
    snprintf(devname, I2C_DEVNAME_FMTLEN, I2C_DEVNAME_FMT, bus);

    dev = dn_bind(devname);

    return dev;
}

int i2c_bus_initialize(int bus)
{
    struct i2c_master_s *dev = i2c_bus_get(bus);

    if (!dev) {
        return -1;
    }

#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    dev->sem_excl = xSemaphoreCreateBinary();

    /* This semaphore is used for signaling and, hence, should not have
    * priority inheritance enabled.
    */
    dev->sem_isr = xSemaphoreCreateBinary();
    xSemaphoreGive(dev->sem_excl);
#else

    dev->flag_excl = 0x01;
    dev->flag_isr = 0x00;
#endif

    return dev->ops->setup(dev);
}

int i2c_dev_lock(struct i2c_master_s *dev)
{
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    return xSemaphoreTake(dev->sem_excl, 1000);
#else

    if (dev->flag_excl == 0x01) {
        dev->flag_excl = 0x00;
        return DTRUE;
    } else {
        return DFALSE;
    }
#endif
}

int i2c_dev_unlock(struct i2c_master_s *dev)
{
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    return xSemaphoreGive(dev->sem_excl);
#else

    dev->flag_excl = 0x01;
    return DTRUE;
#endif
}

int i2c_dev_transfer_wait(struct i2c_master_s *dev, uint32_t timeout)
{
    int ret = DTRUE;

#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    do {
        /* Take the semaphore (perhaps waiting) */
        ret = xSemaphoreTake(dev->sem_isr, 1000);
    } while (ret == pdFALSE);

    return ret;
#else

    uint32_t timeout_ms = 20;
    uint32_t time_start = dn_timems();
    do {
        if (dev->flag_isr == 0x01) {
            dev->flag_isr = 0x00;
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

int i2c_dev_transfer_completed(struct i2c_master_s *dev)
{
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    BaseType_t h_pri;
    xSemaphoreGiveFromISR(dev->sem_isr, &h_pri);
    portYIELD_FROM_ISR(h_pri);
#else

    dev->flag_isr = 0x01;
    return DTRUE;
#endif
}
