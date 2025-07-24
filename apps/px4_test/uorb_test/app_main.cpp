#include <board_config.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <drivers/drv_hrt.h>
#include <pthread.h>
#include "uORBTest_UnitTest.hpp"
#include "uORB/uORBCommon.hpp"

extern "C" {
extern __EXPORT int uorb_main(int argc, char *argv[]);
}

int
uorb_tests_main(int argc, char *argv[])
{
	/*
	 * Test the driver/device.
	 */
	if (argc == 1) {
        PX4_INFO("Test the driver/device");

		uORBTest::UnitTest &t = uORBTest::UnitTest::instance();
		int rc = t.test();

		if (rc == OK) {
			PX4_INFO("PASS");
			return 0;

		} else {
			PX4_ERR("FAIL");
			return -1;
		}
	}

	/*
	 * Test the latency.
	 */
	if (argc > 1 && !strcmp(argv[1], "latency_test")) {
        PX4_INFO("Test the latency");

		uORBTest::UnitTest &t = uORBTest::UnitTest::instance();

		if (argc > 2 && !strcmp(argv[2], "medium")) {
			return t.latency_test<orb_test_medium_s>(ORB_ID(orb_test_medium), true);

		} else if (argc > 2 && !strcmp(argv[2], "large")) {
			return t.latency_test<orb_test_large_s>(ORB_ID(orb_test_large), true);

		} else {
			return t.latency_test<orb_test_s>(ORB_ID(orb_test), true);
		}
	}

	PX4_INFO("Usage: uorb_tests [latency_test]");
	return -EINVAL;
}

char cmd_p[] = "latency_test";
char cmd[] = "uorb_test";
char *t_argv_list[10];

__attribute__((aligned(4), section(".noncacheable"))) volatile uint32_t atomic_counter = 0;
__attribute__((aligned(4), section(".noncacheable"))) volatile uint32_t pri = 0;
__attribute__((aligned(4), section(".noncacheable"))) volatile uint32_t d1 = 0;

void* g_start(void *p)
{
    board_bsp_init();
    hrt_init();
    pthread_setname_np(pthread_self(), "g_start");

    char *tmp_argv[5] = {
        "uorb",
        "start"
    };

    d1 = __atomic_load_n(&atomic_counter, __ATOMIC_SEQ_CST);
	__atomic_store(&d1, &atomic_counter, __ATOMIC_SEQ_CST);

    printf("start call __atomic_fetch_add %x\r\n", (uint32_t)&atomic_counter);
    SCB_CleanDCache_by_Addr((uint32_t*)&atomic_counter, sizeof(atomic_counter));

    // atomic used in Cacheable Memory can trigge HardFault
    pri = __atomic_fetch_add(&atomic_counter, 10, __ATOMIC_SEQ_CST);
    printf("after call __atomic_fetch_add \r\n");


    uorb_main(2, tmp_argv);

    t_argv_list[0] = &cmd[0];
    // t_argv_list[1] = &cmd_p[0];

    uorb_tests_main(1, t_argv_list);

    return NULL;
}

int main(void)
{
    board_init();

    pthread_t tid;
    pthread_attr_t tmpattr = {
        .stackaddr = NULL,
        .stacksize = 2048*sizeof(StackType_t),
        .inheritsched = SCHED_FIFO,
        .schedparam = {
            .sched_priority = 1,
        },
        .detachstate = PTHREAD_CREATE_DETACHED,
    };
    int rv = pthread_create(&tid, &tmpattr, &g_start, NULL);

    sched_start();
    for (;;);
}
