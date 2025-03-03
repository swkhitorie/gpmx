#ifndef APP_MAIN_H_
#define APP_MAIN_H_

#include "board_config.h"
#include "app_macros.h"

#if RTOS_SELECT == RTOS_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#endif

void debug_led_toggle();

#endif

