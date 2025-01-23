#ifndef IST8310_TEST_H_
#define IST8310_TEST_H_

#include <board_config.h>


__attribute__((weak)) void icm42688_write_register(uint8_t addr, uint8_t data);

__attribute__((weak)) void icm42688_read_register(uint8_t addr, uint8_t *buf, uint8_t len, int rwway);

#endif
