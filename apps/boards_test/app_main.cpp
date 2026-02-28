#include <board_config.h>

#ifndef CONFIG_FREERTOS_ENABLE
int main(int argc, char **argv)
#else
extern "C" {
void main_root(void *p)
#endif
{
    board_test();
}
#ifdef CONFIG_FREERTOS_ENABLE
}
#endif
