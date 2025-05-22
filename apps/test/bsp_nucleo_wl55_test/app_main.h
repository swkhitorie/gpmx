#ifndef APP_MAIN_H_
#define APP_MAIN_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <board_config.h>
#include <app_subghz.h>
#include "radio.h"

#define RADIO_ENABLE_DEBUG
#define RADIO_ENABLE_SENDER_DISPLAY

#ifdef RADIO_ENABLE_DEBUG
#define RADIO_DEBUG(...) printf(__VA_ARGS__)
#endif

#ifdef RADIO_ENABLE_SENDER_DISPLAY
#define RADIO_SENDER_DISPLAY(p,z) fwrite(p, 1, z, stdout)
#endif

#endif
