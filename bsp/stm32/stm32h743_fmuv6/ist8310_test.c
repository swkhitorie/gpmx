#include "ist8310_test.h"

void wait_block()
{
    HAL_Delay(2);
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
