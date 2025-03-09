#ifndef DEV_OPS_I2C_MASTER_H_
#define DEV_OPS_I2C_MASTER_H_

#include <stdint.h>
#include <stdbool.h>

#define I2C_M_WRITE          0x0001
#define I2C_M_READ           0x0002

#define I2C_SPEED_STANDARD   100000  /* Standard speed (100Khz) */
#define I2C_SPEED_FAST       400000  /* Fast speed     (400Khz) */
#define I2C_SPEED_FAST_PLUS  1000000 /* Fast+ speed    (  1Mhz) */
#define I2C_SPEED_HIGH       3400000 /* High speed     (3.4Mhz) */

struct i2c_msg_s
{
    uint16_t addr;       /* Slave address (7- or 10-bit) */
    uint16_t flags;      /* See I2C_M_* definitions */
    uint8_t *buffer;     /* Buffer to be transferred */
    size_t length;       /* Length of the buffer in bytes */
};

struct i2c_transfer_s
{
    struct i2c_msg_s *msgv;  /* Array of I2C messages for the transfer */
    size_t msgc;             /* Number of messages in the array. */
};

struct i2c_ops_s
{
    int (*transfer)(struct i2c_master_s *dev, struct i2c_msg_s *msgs, int count);

    int (*reset)(struct i2c_master_s *dev);
};

struct i2c_master_s
{
    const struct i2c_ops_s *ops;
};

#endif
