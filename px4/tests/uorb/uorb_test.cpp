
#include "uorb_test.hpp"

char cmd_p[] = "latency_test";
char cmd[] = "uorb_test";
char *t_argv_list[10];

__attribute__((aligned(4), section(".noncacheable"))) volatile uint32_t atomic_counter = 0;
__attribute__((aligned(4), section(".noncacheable"))) volatile uint32_t pri = 0;
__attribute__((aligned(4), section(".noncacheable"))) volatile uint32_t d1 = 0;

void uorb_test_entry();
{
    char *tmp_argv[5] = {
        (char *)"uorb",
        (char *)"start"
    };

    d1 = __atomic_load_n(&atomic_counter, __ATOMIC_SEQ_CST);
	__atomic_store(&d1, &atomic_counter, __ATOMIC_SEQ_CST);

    TEST_PRINTF("start call __atomic_fetch_add %x\r\n", (uint32_t)&atomic_counter);
    SCB_CleanDCache_by_Addr((uint32_t*)&atomic_counter, sizeof(atomic_counter));

    // atomic used in Cacheable Memory can trigge HardFault
    pri = __atomic_fetch_add(&atomic_counter, 10, __ATOMIC_SEQ_CST);
    TEST_PRINTF("after call __atomic_fetch_add \r\n");

    uorb_main(2, tmp_argv);

    t_argv_list[0] = &cmd[0];
    // t_argv_list[1] = &cmd_p[0];

    uorb_tests_main(1, t_argv_list);
}

