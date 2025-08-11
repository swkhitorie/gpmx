#ifndef DEV_OPS_I2C_MASTER_H_
#define DEV_OPS_I2C_MASTER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#if defined(CONFIG_BOARD_FREERTOS_ENABLE) && defined(CONFIG_I2C_TASKSYNC)
#include <FreeRTOS.h>
#include <semphr.h>
#endif

/* I2C address calculation.  Convert 7- and 10-bit address to 8-bit and
 * 16-bit read/write address
 */

#define I2C_READBIT          0x01

/* Convert 7- to 8-bit address */

#define I2C_ADDR8(a)         ((a) << 1)
#define I2C_WRITEADDR8(a)    I2C_ADDR8(a)
#define I2C_READADDR8(a)     (I2C_ADDR8(a) | I2C_READBIT)

/* Convert 10- to 16-bit address */

#define I2C_ADDR10H(a)       (0xf0 | (((a) >> 7) & 0x06))
#define I2C_ADDR10L(a)       ((a) & 0xff)

#define I2C_WRITEADDR10H(a)  I2C_ADDR10H(a)
#define I2C_WRITEADDR10L(a)  I2C_ADDR10L(a)

#define I2C_READADDR10H(a)   (I2C_ADDR10H(a) | I2C_READBIT)
#define I2C_READADDR10L(a)   I2C_ADDR10L(a)

/* Bit definitions for the flags field in struct i2c_msg_s
 *
 * START/STOP Rules:
 *
 * 1. The lower half I2C driver will always issue the START condition at the
 *    beginning of a message unless I2C_M_NOSTART flag is set in the
 *    message.
 *
 * 2. The lower half I2C driver will always issue the STOP condition at the
 *    end of the messages unless:
 *
 *    a. The I2C_M_NOSTOP flag is set in the message, OR
 *    b. The following message has the I2C_M_NOSTART flag set (meaning
 *       that following message is simply a continuation of the transfer).
 *
 * A proper I2C repeated start would then have I2C_M_NOSTOP set on msg[n]
 * and I2C_M_NOSTART *not* set on msg[n+1].  See the following table:
 *
 *   msg[n].flags  msg[n+1].flags Behavior
 *   ------------ --------------- -----------------------------------------
 *   0            0                Two normal, separate messages with STOP
 *                                 on msg[n] then START on msg[n+1]
 *   0*           I2C_M_NOSTART    Continuation of the same transfer (must
 *                                 be the same direction).  See NOTE below.
 *   NO_STOP      0                No STOP on msg[n]; repeated START on
 *                                 msg[n+1].
 *
 * * NOTE: NO_STOP is implied in this case and may or not be explicitly
 *   included in the msg[n] flags
 */

#define I2C_M_READ           0x0001 /* Read data, from slave to master */
#define I2C_M_TEN            0x0002 /* Ten bit address */
#define I2C_M_NOSTOP         0x0040 /* Message should not end with a STOP */
#define I2C_M_NOSTART        0x0080 /* Message should not begin with a START */

/* I2c bus speed */

#define I2C_SPEED_STANDARD   100000  /* Standard speed (100Khz) */
#define I2C_SPEED_FAST       400000  /* Fast speed     (400Khz) */
#define I2C_SPEED_FAST_PLUS  1000000 /* Fast+ speed    (  1Mhz) */
#define I2C_SPEED_HIGH       3400000 /* High speed     (3.4Mhz) */

/****************************************************************************
 * Name: I2C_TRANSFER_BLOCK
 *
 * Description:
 *   Perform a sequence of I2C transfers, each transfer is started with a
 *   START and the final transfer is completed with a STOP. Each sequence
 *   will be an 'atomic' operation in the sense that any other I2C actions
 *   will be serialized and pend until this sequence of transfers completes.
 *
 * Input Parameters:
 *   dev   - Device-specific state data
 *   msgs  - A pointer to a set of message descriptors
 *   count - The number of transfers to perform
 *
 * Returned Value:
 *   Zero (OK) or positive on success; a negated errno value on failure.
 *
 *   Note : some implementations of this interface return the number of
 *          transfers completed, but others return OK on success.
 *
 ****************************************************************************/

#define I2C_TRANSFER(d,m,c) ((d)->ops->transfer(d,m,c))

/************************************************************************************
 * Name: I2C_RESET
 *
 * Description:
 *   Perform an I2C bus reset in an attempt to break loose stuck I2C devices.
 *
 * Input Parameters:
 *   dev   - Device-specific state data
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno value on failure.
 *
 ************************************************************************************/

#define I2C_RESET(d) ((d)->ops->reset(d))

/* The I2C lower half driver interface */

struct i2c_master_s;
struct i2c_msg_s;
struct i2c_ops_s
{
  int (*setup)(struct i2c_master_s *dev);

  int (*transfer)(struct i2c_master_s *dev,
                    struct i2c_msg_s *msgs, int count);

  int (*reset)(struct i2c_master_s *dev);
};

/* This structure contains the full state of I2C as needed for a specific
 * transfer.  It is passed to I2C methods so that I2C transfer may be
 * performed in a thread safe manner.
 */

struct i2c_config_s
{
  uint32_t frequency;          /* I2C frequency */
  uint16_t address;            /* I2C address (7- or 10-bit) */
  uint8_t addrlen;             /* I2C address length (7 or 10 bits) */
};

/* I2C transaction segment beginning with a START. A number of these can
 * be transferred together to form an arbitrary sequence of write/read transfer
 * to an I2C slave device.
 */

struct i2c_msg_s
{
  uint32_t frequency;     /* I2C frequency */
  uint16_t addr;          /* Slave address (7- or 10-bit) */
  uint16_t flags;         /* See I2C_M_* definitions */
  uint16_t length;         /* Length of the buffer in bytes */
  uint8_t *buffer;        /* Buffer to be transferred */
  uint8_t __pad[1];
};

/* I2C private data.  This structure only defines the initial fields of the
 * structure visible to the I2C client.  The specific implementation may
 * add additional, device specific fields after the vtable.
 */

struct i2c_master_s
{
#if defined(CONFIG_BOARD_FREERTOS_ENABLE) && defined(CONFIG_I2C_TASKSYNC)
  SemaphoreHandle_t  sem_excl;    /* Mutual exclusion semaphore */
  SemaphoreHandle_t  sem_isr;     /* Interrupt wait semaphore */
#else
  volatile uint32_t flag_excl;
  volatile uint32_t flag_isr;
#endif

  struct i2c_config_s cfg;

  /* Driver interface */
  const struct i2c_ops_s  *ops;  /* Arch-specific operations */
  void                    *priv; /* Used by the arch-specific logic */
};

/* This structure is used to communicate with the I2C character driver in
 * order to perform IOCTL transfers.
 */

struct i2c_transfer_s
{
  struct i2c_msg_s *msgv; /* Array of I2C messages for the transfer */
  size_t msgc;                /* Number of messages in the array. */
};

#if defined(__cplusplus)
extern "C"{
#endif

int                  i2c_register(struct i2c_master_s *dev, int bus);
struct i2c_master_s* i2c_bus_get(int bus);
int                  i2c_bus_initialize(int bus);

int  i2c_dev_lock(struct i2c_master_s *dev);
int  i2c_dev_unlock(struct i2c_master_s *dev);
int  i2c_dev_transfer_wait(struct i2c_master_s *dev, uint32_t timeout);
int  i2c_dev_transfer_completed(struct i2c_master_s *dev);

#if defined(__cplusplus)
}
#endif


#endif
