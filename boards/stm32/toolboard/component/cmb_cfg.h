/*
 * This file is part of the CmBacktrace Library.
 *
 * Copyright (c) 2016, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: It is the configure head file for this library.
 * Created on: 2016-12-15
 */

#ifndef _CMB_CFG_H_
#define _CMB_CFG_H_

/* print line, must config by user */
#include "board_config.h"
#define cmb_println(...)               BOARD_CRUSH_PRINTF(__VA_ARGS__); BOARD_CRUSH_PRINTF("\r\n")

/**
#define CMB_CPU_ARM_CORTEX_M0             0
#define CMB_CPU_ARM_CORTEX_M3             1
#define CMB_CPU_ARM_CORTEX_M4             2
#define CMB_CPU_ARM_CORTEX_M7             3
#define CMB_CPU_ARM_CORTEX_M33            4

#define CMB_OS_PLATFORM_RTT               0
#define CMB_OS_PLATFORM_UCOSII            1
#define CMB_OS_PLATFORM_UCOSIII           2
#define CMB_OS_PLATFORM_FREERTOS          3

#define CMB_PRINT_LANGUAGE_ENGLISH        0
#define CMB_PRINT_LANGUAGE_CHINESE        1
#define CMB_PRINT_LANGUAGE_CHINESE_UTF8   2
 */

#ifndef CONFIG_BOARD_FREERTOS_ENABLE

/* enable bare metal(no OS) platform */
#define CMB_USING_BARE_METAL_PLATFORM
#else
/* enable OS platform */
#define CMB_USING_OS_PLATFORM

/* OS platform type, must config when CMB_USING_OS_PLATFORM is enable */
#define CMB_OS_PLATFORM_TYPE    3
#endif

/* cpu platform type, must config by user */
#define CMB_CPU_PLATFORM_TYPE          1

/* enable dump stack information */
#define CMB_USING_DUMP_STACK_INFO

/* language of print information */
#define CMB_PRINT_LANGUAGE             0

#endif /* _CMB_CFG_H_ */
