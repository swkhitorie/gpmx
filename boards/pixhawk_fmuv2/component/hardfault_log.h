#ifndef HARDFAULT_LOG_H_
#define HARDFAULT_LOG_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void hardfault_log_start();
void hardfault_log_end();
void hardfault_log_write(const char *format, ...);
void hardfault_log_clear();
uint8_t *hardfault_log_check();

#ifdef __cplusplus
}
#endif

#endif
