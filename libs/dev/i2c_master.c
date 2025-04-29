#include <dev/i2c_master.h>

int i2c_register(struct i2c_master_s *dev, int bus)
{

    return 0;
}

#if defined(CONFIG_BOARD_FREERTOS_ENABLE) && defined(CONFIG_I2C_TASKSYNC)

void i2c_sem_init(struct i2c_master_s *dev)
{
    dev->sem_excl = xSemaphoreCreateBinary();

    /* This semaphore is used for signaling and, hence, should not have
    * priority inheritance enabled.
    */
    dev->sem_isr = xSemaphoreCreateBinary();
    xSemaphoreTake(dev->sem_isr, 1);
}

void i2c_sem_destroy(struct i2c_master_s *dev)
{
    vSemaphoreDelete(dev->sem_excl);
    vSemaphoreDelete(dev->sem_isr);
}

void i2c_sem_wait(struct i2c_master_s *dev)
{
    int ret;
    do {
        /* Take the semaphore (perhaps waiting) */
        ret = xSemaphoreTake(dev->sem_excl, portMAX_DELAY);
    } while (ret == pdFALSE);
}

void i2c_sem_post(struct i2c_master_s *dev)
{
    xSemaphoreGive(dev->sem_excl);
}

#endif
