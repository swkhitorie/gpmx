#include <board_config.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(CONFIG_FREERTOS_ENABLE) && !defined(CONFIG_RTTNANO_ENABLE)
int main(int argc, char **argv)
#else
void main_root(void *p)
#endif
{
    board_test();
}

#ifdef __cplusplus
}
#endif
