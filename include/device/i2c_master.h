#ifndef DEV_OPS_I2C_MASTER_H_
#define DEV_OPS_I2C_MASTER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#if defined(CONFIG_BOARD_FREERTOS_ENABLE) && defined(CONFIG_I2C_TASKSYNC)
#include <FreeRTOS.h>
#include <semphr.h>
#endif

#define I2C_M_WRITE          0x0001
#define I2C_M_READ           0x0002
#define I2C_REG_WRITE        0x0003
#define I2C_REG_READ         0x0004

#define I2C_ADDR_7BIT          0x0010
#define I2C_ADDR_10BIT         0x0011

#define I2C_REG_SZ_8BIT        0x01
#define I2C_REG_SZ_16BIT       0x02

#define I2C_SPEED_STANDARD   100000  /* Standard speed (100Khz) */
#define I2C_SPEED_FAST       400000  /* Fast speed     (400Khz) */
#define I2C_SPEED_FAST_PLUS  1000000 /* Fast+ speed    (  1Mhz) */
#define I2C_SPEED_HIGH       3400000 /* High speed     (3.4Mhz) */

#define I2C_TRANSFER(d,m,c) ((d)->ops->transfer(d,m,c))

#define I2C_TRANSFERIT(d,m,c) ((d)->ops->transferit(d,m,c))

#define I2C_RESET(d) ((d)->ops->reset(d))

struct i2c_msg_s
{
    uint16_t addr;       /* Slave address (7- or 10-bit) */
    uint8_t reg_sz;      /* Register size (1byte or 2byte) */
    uint16_t flags;      /* See I2C_M_* definitions */
    uint8_t *xbuffer;     /* Buffer to be transferred */
    size_t xlength;
    uint8_t *rbuffer;    /* Buffer to be received */
    size_t rlength;       
};

struct i2c_transfer_s
{
    struct i2c_msg_s *msgv;  /* Array of I2C messages for the transfer */
    size_t msgc;             /* Number of messages in the array. */
};

struct i2c_master_s;
struct i2c_ops_s
{
    int (*setup)(struct i2c_master_s *dev);

    int (*transfer)(struct i2c_master_s *dev, struct i2c_msg_s *msgs, int count);

    int (*transferit)(struct i2c_master_s *dev, struct i2c_msg_s *msgs, int count);

    int (*reset)(struct i2c_master_s *dev);
};

struct i2c_master_s
{
    int clk_speed;

    uint8_t addr_mode;

    uint8_t msgc;                /* Message count */
    uint8_t msgi;                /* Message index */
    struct i2c_msg_s *msgv;      /* Message list */

#if defined(CONFIG_BOARD_FREERTOS_ENABLE) && defined(CONFIG_I2C_TASKSYNC)
    SemaphoreHandle_t  mutex;       /* Prevent devices from being occupied by multiple threads */
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
     * void i2c_transfer_isr_callback() {
     *     sem_post(&sem_isr)
     * }
     */
    SemaphoreHandle_t  sem_excl;    /* Mutual exclusion semaphore */
    SemaphoreHandle_t  sem_isr;     /* Interrupt wait semaphore */
#endif

    /* Driver interface */
    const struct i2c_ops_s  *ops;  /* Arch-specific operations */
    void                    *priv; /* Used by the arch-specific logic */
};

#if defined(__cplusplus)
extern "C"{
#endif

int i2c_register(struct i2c_master_s *dev, int bus);

#if defined(CONFIG_BOARD_FREERTOS_ENABLE) && defined(CONFIG_I2C_TASKSYNC)

void i2c_sem_init(struct i2c_master_s *dev);

void i2c_sem_destroy(struct i2c_master_s *dev);

void i2c_sem_wait(struct i2c_master_s *dev);

void i2c_sem_post(struct i2c_master_s *dev);

#endif

#if defined(__cplusplus)
}
#endif


#endif
