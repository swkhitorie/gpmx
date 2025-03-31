#include "ist8310_test.h"
#include <dev/i2c_master.h>

struct i2c_master_s *seni;

void wait_block()
{
    HAL_Delay(2);
}

struct i2c_msg_s smsg;
struct i2c_msg_s rmsg;
uint8_t datat[2];
uint8_t datatr;
void ist8310_write_register(uint8_t addr, uint8_t data)
{
    int ret = 0;
    datat[0] = addr;
    datat[1] = data;

    smsg.flags = I2C_REG_WRITE;
    smsg.addr = 0x0c<<1;
    smsg.reg_sz = 1;
    smsg.xbuffer = datat;
    smsg.xlength = 2;

	ret = I2C_TRANSFER(seni,&smsg,1);
}

void ist8310_read_register(uint8_t addr, uint8_t *buf, uint8_t len, int rwway)
{
	int ret = 0;
    datatr = addr;
    rmsg.flags = I2C_REG_READ;
    rmsg.addr = 0x0c<<1;
    rmsg.reg_sz = 1;
    rmsg.xbuffer = &datatr;
    rmsg.xlength = 1;
    rmsg.rbuffer = buf;
    rmsg.rlength = len;

    ret = I2C_TRANSFERIT(seni,&rmsg,1);
    return;
}


bool ist8310_init()
{
    int idx;
    uint16_t reg_write_list[2] = {
        ADDR_CTRL3, ADDR_CTRL4,
    };
    uint8_t reg_write_val[2] = {
        CTRL3_SAMPLEAVG_16, CTRL4_SRPD,
    };
    uint8_t reg_write_check_val[2] = {0x00};

    seni = dbind("/sensor_i2c");
    if (seni == NULL) {
        printf("not get i2c handle");
        return false;
    }
	/* software reset */
	ist8310_write_register(ADDR_CTRL2, CTRL2_SRST);
    wait_block();

    for (int i = 0; i < 2; i++) {
        ist8310_write_register(reg_write_list[i], reg_write_val[i]);
        wait_block();
    }

    for (idx = 0; idx < 2; idx++) {
        ist8310_read_register(reg_write_list[idx], &reg_write_check_val[idx], 1, 0);
        wait_block();
        if (reg_write_check_val[idx] != reg_write_val[idx]) {
            break;
        }
    }
    if (idx != 2) {
        printf("[ist8310] check false %d \r\n", idx);
        return false;
    }

	uint8_t tmpid = 0;
    ist8310_read_register(ADDR_WAI, &tmpid, 1, 0);
    printf("[ist8310] chip id %x \r\n", tmpid);
    return (tmpid == WAI_EXPECTED_VALUE);
}

uint8_t rawmag[6] = {0};
bool ist8310_mag(int16_t *mag)
{
    ist8310_write_register(ADDR_CTRL1, CTRL1_MODE_SINGLE);
    ist8310_read_register(ADDR_DATA_OUT_X_LSB, &rawmag[0], 6, 0);
	mag[0] = (int16_t)((rawmag[1] << 8) | rawmag[0]);
	mag[1] = (int16_t)((rawmag[3] << 8) | rawmag[2]);
	mag[2] = (int16_t)((rawmag[5] << 8) | rawmag[4]);
}
