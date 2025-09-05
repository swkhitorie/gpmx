#ifndef APP_MAIN_H_
#define APP_MAIN_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <board_config.h>

extern volatile uint32_t   store_app_addr;
extern uint32_t            ybin_type;
extern uint32_t            ybin_version;
extern uint32_t            crc_file;
extern int                 total_ysize;

#endif
