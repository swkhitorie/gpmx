#ifndef __PX_PLATFORM_H_
#define __PX_PLATFORM_H_

#include <stdint.h>
#include "platform_common/log.h"
#include "driver/drv_hrt.h"
#include "workqueue/workqueue_manager.h"
#include "wqueue/wqueue.h"
#include "uorb/uorb_common.h"
#include "uorb/uorb_manager.h"
#include "libs/parameters/param.h"

#ifdef __cplusplus
extern "C" {
#endif

void px_platform_init();

#ifdef __cplusplus
}
#endif

#endif
