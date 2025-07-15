#pragma once

#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>

#define px4_clock_gettime clock_gettime
#define px4_clock_settime clock_settime
#define px4_usleep usleep
#define px4_sleep sleep
#define px4_pthread_cond_timedwait pthread_cond_timedwait


