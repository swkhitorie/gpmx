#ifndef DEV_OPS_I2C_MASTER_H_
#define DEV_OPS_I2C_MASTER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define I2C_M_WRITE          0x0001
#define I2C_M_READ           0x0002
#define I2C_REG_WRITE        0x0003
#define I2C_REG_READ         0x0004

#define I2C_ADDR_7BIT          0x0010
#define I2C_ADDR_10BIT         0x0011

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
    uint8_t reg_sz;
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

    /* Driver interface */
    const struct i2c_ops_s  *ops;  /* Arch-specific operations */
    void                    *priv; /* Used by the arch-specific logic */
};

#ifdef cplusplus
extern "C" {
#endif

int i2c_register(struct i2c_master_s *i2c, int bus);

#ifdef cplusplus
}
#endif


#endif
