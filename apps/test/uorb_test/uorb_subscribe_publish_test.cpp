#include <board_config.h>
#include <drivers/drv_hrt.h>

#include <stdio.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sched.h>
#include <sdqueue.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <utils.h>
#include <pthread.h>

#include <uorb/topics/sensor_accel.h>
#include "uorb/abs_time.h"
#include "uorb/publication.h"
#include "uorb/publication_multi.h"
#include "uorb/subscription.h"

typedef struct __pthread_test{
    pthread_attr_t attr;
    pthread_t id;
    float arg;
    struct sched_param param;
} pthread_test_t;

pthread_test_t pa1;
pthread_test_t pa2;

void *thread_publisher(void *arg) {
    uorb::PublicationData<uorb::msg::sensor_accel> pub_accel;
    auto &data = pub_accel.get();
    data.x = 1.5f;
    data.y = 1.6f;
    data.z = 3.14f;

    for (int i = 0; i < 10; i++) {

        if (!pub_accel.Publish()) {
            printf("Publish error \n");
        }

        data.x += 0.1f;
        data.y += 2.6f;
        data.z -= 3.14f;

        printf("publisher running \n");
        sleep(1000);
    }
    printf("Publication over. \n");
    return nullptr;
}


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) \
    ((int)((sizeof(array) / sizeof((array)[0]))))
#endif


void *thread_subscriber(void *unused) {
    uorb::SubscriptionData<uorb::msg::sensor_accel> sub_accel;

    int timeout_ms = 2000;

    struct orb_pollfd poll_fds[] = {
        {.fd = sub_accel.handle(), .events = POLLIN}
    };

    while (true) {
        if (0 < orb_poll(poll_fds, ARRAY_SIZE(poll_fds), timeout_ms)) {
            if (sub_accel.Update()) {
                auto data = sub_accel.get();
                printf("Sub: %.3f, %.3f, %.3f \n", data.x, data.y, data.z);
            }
        } else {
            printf("Got no data within %d milliseconds \n", 2000);
            break;
        }
    }

    printf("subscription over \n");
    return nullptr;
}

int main(void)
{
    board_init();
    hrt_init();

    int rv = pthread_create(&pa1.id, nullptr, thread_publisher, nullptr);
    if (rv != 0) {
        fprintf(stdout, "[] create pthread failed\r\n");
    }

    rv = pthread_create(&pa2.id, nullptr, thread_subscriber, nullptr);
    if (rv != 0) {
        fprintf(stdout, "[] create pthread failed\r\n");
    }

    sched_start();
    for (;;);
}
