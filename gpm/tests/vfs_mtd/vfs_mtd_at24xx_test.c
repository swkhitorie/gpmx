#include <gpmx/config.h>
#include <mlog.h>
#include <string.h>

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <gpm/fs/fs.h>

#include <board_config.h>
#include <gpm/drivers/drivers.h>
#include <gpm/mtd/mtd.h>

static void* vfs_mtd_at24xx_test_process(void *p);
int vfs_mtd_at24xx_test(int argc, char **argv)
{
    int task_id;
    pthread_attr_t attr;
    pthread_t id;
    pthread_attr_init(&attr);

    struct sched_param attr_param;
    attr_param.sched_priority = sched_get_priority_max(SCHED_FIFO) / 2;
    pthread_attr_setschedparam(&attr, &attr_param);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setstacksize(&attr, 512*sizeof(void *));

    task_id = pthread_create(&id, &attr, &vfs_mtd_at24xx_test_process, NULL);

    return 0;
}

uint32_t wd_value = 0xAABBCCDD;
uint32_t rd_value = 0;
void* vfs_mtd_at24xx_test_process(void *p)
{
    int task_id;
    pthread_attr_t attr;
    pthread_t id;
    pthread_attr_init(&attr);

    pthread_setname_np(pthread_self(), "usr_test");

    int rv = 0;
    int ret = 0;
    int fd = 0;

    struct mtd_dev_s * mtd_instance = at24c_initialize(g_at24xx_dev);
    if (!mtd_instance) {
        KMINFO("failed in at25c\r\n");
    }
    rv = ftl_initialize(0, mtd_instance);
    if (rv < 0) {
        KMINFO("ftl_initialize failed \r\n");
        return NULL;
    }

    rv = bchdev_register("/dev/mtdblock0", "/fs/mtd_params", false);
    if (rv < 0) {
        KMINFO("bchdev_register failed \r\n");
        return NULL;
    }

    while (1) {

        KMRAW("\r\n\r\n[at24c] start write 4bytes\r\n");
        fd = open("/fs/mtd_params", O_RDOK | O_WROK | O_CREAT);
        if (fd < 0) {
            KMINFO("failed to open /fs/mtd_params \r\n");
        }
        ret = write(fd, &wd_value, 4);
        if (ret < 0) {
            KMINFO("write failed \r\n");
        } else {
            KMINFO("write success \r\n");
        }
        close(fd);


        KMRAW("\r\n\r\n[at24c] start read 4bytes\r\n");
        fd = open("/fs/mtd_params", O_RDOK | O_WROK | O_CREAT);
        if (fd < 0) {
            KMINFO("failed to open /fs/mtd_params \r\n");
        }
        ret = read(fd, &rd_value, 4);
        if (ret < 0) {
            KMINFO("read failed \r\n");
        } else {
            KMINFO("read success \r\n");
        }
        if (rd_value != wd_value) {
            KMINFO("not same:%x %x \r\n", rd_value, wd_value);
            close(fd);
            return NULL;
        } else {
            KMINFO("same:%x %x \r\n", rd_value, wd_value);
            close(fd);
            return NULL;
        }
        close(fd);

        sleep(2);
    }

    return NULL;
}
