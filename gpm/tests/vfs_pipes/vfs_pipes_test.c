#include <gpmx/config.h>
#include <mlog.h>
#include <string.h>

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <gpm/fs/fs.h>

static void* vfs_pipes_wr_test_process(void *p);
int vfs_pipes_test(int argc, char **argv)
{
    int task_id;
    pthread_attr_t attr;
    pthread_t id;
    pthread_attr_init(&attr);

#if 1
    struct sched_param attr_param;
    attr_param.sched_priority = sched_get_priority_max(SCHED_FIFO) / 2;
    pthread_attr_setschedparam(&attr, &attr_param);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setstacksize(&attr, 512*sizeof(void *));
#else
    attr.schedparam.sched_priority = sched_get_priority_max(SCHED_FIFO) / 2;
    attr.detachstate = PTHREAD_CREATE_JOINABLE;
    attr.stacksize = 512 * 4;
#endif

    task_id = pthread_create(&id, &attr, &vfs_pipes_wr_test_process, NULL);

    return 0;
}

static int fd_t[2];
static int pipe_ready = 0;
static void* vfs_pipes_rd_test_process(void *p);
void* vfs_pipes_wr_test_process(void *p)
{

    int task_id;
    pthread_attr_t attr;
    pthread_t id;
    pthread_attr_init(&attr);

    pthread_setname_np(pthread_self(), "pipe_wd");

#if 1
    struct sched_param attr_param;
    attr_param.sched_priority = sched_get_priority_max(SCHED_FIFO) / 2;
    pthread_attr_setschedparam(&attr, &attr_param);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setstacksize(&attr, 512*sizeof(void *));
#else
    attr.schedparam.sched_priority = sched_get_priority_max(SCHED_FIFO) / 2;
    attr.detachstate = PTHREAD_CREATE_JOINABLE;
    attr.stacksize = 512 * 4;
#endif
    const char *test_msg = "Hello from pipe writer!";
    ssize_t written;
    int ret = pipe2(fd_t, O_RDOK | O_WROK | O_CREAT);
    if (ret < 0) {
        KMINFO("pipe2 failed \n");
        return NULL;
    }
    KMINFO("Pipe created, read fd=%d, write fd=%d\n", fd_t[0], fd_t[1]);

    pipe_ready = 1;

    const char *test_string = "hello stdout\r\n";

    task_id = pthread_create(&id, &attr, &vfs_pipes_rd_test_process, NULL);
    KMINFO("Writer thread started, will write test messages\n");

    for (int i = 0; i < 5; i++) {
        char msg[64];
        snprintf(msg, sizeof(msg), "%s #%d", test_msg, i);
        size_t len = strlen(msg) + 1;
        write(STDOUT_FILENO, test_string, strlen(test_string));

        KMINFO("Writing: \"%s\" (len=%zu)\n", msg, len);
        written = write(fd_t[1], msg, len);
        if (written < 0) {
            KMINFO("Write failed: %s\n", errno);
            break;
        } else if (written != (ssize_t)len) {
            KMINFO("Write partial: %zd of %zu bytes\n", written, len);
        } else {
            KMINFO("Write success: %zd bytes\n", written);
        }

        sleep(1);
    }

    KMINFO("Writer finished, closing write end\n");
    close(fd_t[1]);

    pthread_join(id, NULL);
    KMINFO("Reader thread joined\n");

    return NULL;
}

void *vfs_pipes_rd_test_process(void *p)
{
    char buffer[64];
    ssize_t nread;

    pthread_setname_np(pthread_self(), "pipe_rd");
    KMINFO("Reader thread started, waiting for data...\n");

    while (!pipe_ready) {
        sleep(5);
    }

    while (1) {
        nread = read(fd_t[0], buffer, sizeof(buffer) - 1);
        if (nread < 0) {
            KMINFO("Read error: %d\n", errno);
            break;
        } else if (nread == 0) {
            KMINFO("Read returned 0 (EOF), pipe write end closed\n");
            break;
        } else {
            buffer[nread] = '\0';
            KMINFO("Read %zd bytes: \"%s\"\n", nread, buffer);
            KMINFO("Hex: ");
            for (int i = 0; i < nread; i++) {
                KMRAW("%02x ", (unsigned char)buffer[i]);
            }
            KMRAW("\n");
        }
    }

    KMINFO("Reader thread exiting\n");
    return NULL;
}

