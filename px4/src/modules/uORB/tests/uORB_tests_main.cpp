#include "uORBTest_UnitTest.hpp"
#include "uORB/uORBCommon.hpp"

extern "C" {
extern __EXPORT int uorb_main(int argc, char *argv[]);
}

int
uorb_tests_main(int argc, char *argv[])
{
	// Test the driver/device.

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

	// Test the latency.
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
