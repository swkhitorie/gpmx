#include "./app_main.h"
#include <drivers/drv_hrt.h>
#include <stdio.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sched.h>
#include <sdqueue.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <utils.h>
#include <pthread.h>

/**
 * 1. test simple pthread
 * 2. test time clock delay, like Delay and Delay Until
 * 3. soft timer triggle
 * 4. msg queue debug (deadlock: process poster priority must big than getter process)
 * 5. semaphore test
 * 6. pthread_mutex_t test
 */
#define POSIX_TEST_ITEM      (6)

// 1:Delay 2:Delay Until
#define POSIX_DELAY_METHOD   (2)

void app_posix_freertos_debug_init();

