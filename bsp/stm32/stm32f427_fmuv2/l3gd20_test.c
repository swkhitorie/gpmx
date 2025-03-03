#include "l3gd20_test.h"

void wait_block_1(uint8_t t)
{
    HAL_Delay(t);
}

void l3gd20_init()
{
    uint8_t data;
    l3gd20_read_register(ADDR_WHO_AM_I, &data, 1, 0);
    wait_block_1(2);

    l3gd20_read_register(ADDR_WHO_AM_I, &data, 1, 0);
    wait_block_1(2);

    if (data == WHO_I_AM || data == WHO_I_AM_H || data == WHO_I_AM_L3G4200D) {
        printf("[l3gd20] id: %x \r\n", data);
    } else {
        printf("[l3gd20] unknown id :%x \r\n", data);
    }
}

int l3gd20_read(int16_t *data)
{

}

