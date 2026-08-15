#include <gpmx/config.h>

#include "driver/drv_hrt.h"
#include "mlog/mlog.h"

#include <pthread.h>
#include <time.h>
#include <unistd.h>

int test1_main(int argc, char **argv)
{
    KMINFO("[test1_main] called \r\n");
    return 0;
}

int test2_main(int argc, char **argv)
{
    KMINFO("[test2_main] called \r\n");
    return 0;
}

int test3_main(int argc, char **argv)
{
    KMINFO("[test3_main] called \r\n");
    return 0;
}

int nshlib_test(int argc, char **argv)
{
#if defined(CONFIG_FREERTOS_ENABLE)
    
#elif defined(CONFIG_RTTNANO_ENABLE)

#endif
    return 0;
}
