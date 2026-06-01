#ifndef KERNEL_MODULE_DEFINES_H_
#define KERNEL_MODULE_DEFINES_H_

#include "stdint.h"
#include <board_config.h>

#ifndef KMINFO
#define KMINFO    BOARD_PRINTF
#endif

#ifndef KMERROR
#define KMERROR    BOARD_PRINTF
#endif

#ifndef KMWARN
#define KMWARN    BOARD_PRINTF
#endif

#ifndef KMDEBUG
#define KMDEBUG    BOARD_PRINTF
#endif

#endif
