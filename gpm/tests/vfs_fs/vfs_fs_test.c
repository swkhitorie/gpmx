#include <gpmx/config.h>
#include <mlog.h>
#include <string.h>

#include <driver/drv_hrt.h>

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <gpm/fs/fs.h>

#include <board_config.h>
#include <gpm/drivers/drivers.h>
#include <gpm/mtd/mtd.h>
#include <sys/mount.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <unistd.h>
#include <dirent.h>

int ls(const char *path)
{
    DIR *dir;
    struct dirent *entry;
    struct stat buf;
    char fullpath[64];
    char perm[11];

    dir = opendir(path);
    if (dir == NULL) {
        return -1;
    }

    KMRAW("%s:\n", path);

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
        if (stat(fullpath, &buf) == -1) {
            continue;
        }

        if (S_ISDIR(buf.st_mode)) {
            perm[0] = 'd';
        } else {
            perm[0] = '-';
        }

        perm[1] = (buf.st_mode & S_IRUSR) ? 'r' : '-';
        perm[2] = (buf.st_mode & S_IWUSR) ? 'w' : '-';
        perm[3] = (buf.st_mode & S_IXUSR) ? 'x' : '-';

        perm[4] = (buf.st_mode & S_IRGRP) ? 'r' : '-';
        perm[5] = (buf.st_mode & S_IWGRP) ? 'w' : '-';
        perm[6] = (buf.st_mode & S_IXGRP) ? 'x' : '-';
 
        perm[7] = (buf.st_mode & S_IROTH) ? 'r' : '-';
        perm[8] = (buf.st_mode & S_IWOTH) ? 'w' : '-';
        perm[9] = (buf.st_mode & S_IXOTH) ? 'x' : '-';
        perm[10] = '\0';

        KMRAW(" %s %8lld %s%s\n",
            perm,
            (long long)buf.st_size,
            entry->d_name,
            S_ISDIR(buf.st_mode) ? "/" : "");
    }

    closedir(dir);
    return 0;
}

static void* vfs_fatfs_test_process(void *p);
int vfs_fatfs_test(int argc, char **argv)
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

    task_id = pthread_create(&id, &attr, &vfs_fatfs_test_process, NULL);

    return 0;
}
void* vfs_fatfs_test_process(void *p)
{
    pthread_setname_np(pthread_self(), "usr_test");

    int ret = mount(NULL, "/fs/mmcsd", "vfat", 0, "0:/");
    if (ret < 0) {
        KMINFO("mount /fs/mmcsd failed\r\n");
    }

    int fd = open("/fs/mmcsd/fatfs_unitTest.txt", O_RDONLY);
    if (fd < 0) {
        KMINFO("open /fs/mmcsd/fatfs_unitTest.txt failed \r\n");
    }

    ls("/fs/mmcsd");

    char buf[128];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        write(STDOUT_FILENO, buf, n);
    }

    close(fd);

    while (1) {

        sleep(2);
    }

    return NULL;
}


#define TEST_FILE_COUNT  5
#define TEST_FILE_SIZE   (2 * 1024 * 1024)    /* 每个文件 2MB */
#define BUFFER_SIZE      4096

static uint8_t write_buf[BUFFER_SIZE];
static uint8_t read_buf[BUFFER_SIZE];

void print_size1(uint64_t bytes)
{
    if (bytes >= 1024ULL * 1024 * 1024 * 1024) {
        KMRAW("%.2f TB", (double)bytes / (1024ULL * 1024 * 1024 * 1024));
    } else if (bytes >= 1024 * 1024 * 1024) {
        KMRAW("%.2f GB", (double)bytes / (1024 * 1024 * 1024));
    } else if (bytes >= 1024 * 1024) {
        KMRAW("%.2f MB", (double)bytes / (1024 * 1024));
    } else if (bytes >= 1024) {
        KMRAW("%.2f KB", (double)bytes / 1024);
    } else {
        KMRAW("%llu B", bytes);
    }
}
static void print_fs_capacity(const char *path)
{
    struct statfs buf;
    if (statfs(path, &buf) != 0) {
        KMRAW("statfs error\n");
        return;
    }
    uint64_t total = (uint64_t)buf.f_blocks * buf.f_bsize;
    uint64_t free  = (uint64_t)buf.f_bfree  * buf.f_bsize;

    KMRAW("Total: ");
    print_size1(total);
    KMRAW("\r\n");

    KMRAW("Free: ");
    print_size1(free);
    KMRAW("\r\n");
}
static void* vfs_fatfs_total_test_process(void *p);
int vfs_fatfs_total_test(int argc, char **argv)
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

    task_id = pthread_create(&id, &attr, &vfs_fatfs_total_test_process, NULL);

    return 0;
}

void* vfs_fatfs_total_test_process(void *p)
{
    int ret = 0;
    char file_path[128];
    char test_dir[] = "/fs/mmcsd/test_case";
    char base_dir[] = "/fs/mmcsd";
    int i, j;
    hrt_abstime start, end;
    uint64_t total_write_time = 0, total_read_time = 0;

    pthread_setname_np(pthread_self(), "usr_test");

    ret = mount(NULL, "/fs/mmcsd", "vfat", 0, "0:/");
    if (ret < 0) {
        KMINFO("mount /fs/mmcsd failed\r\n");
    }

    KMRAW("=== Initial directory listing ===\n");
    ls(base_dir);

    KMRAW("=== Filesystem capacity ===\n");
    print_fs_capacity(base_dir);

    struct stat st;
    if (stat(test_dir, &st) == 0) {
        KMRAW("Test directory exists, removing...\n");

        DIR *dir = opendir(test_dir);
        if (dir) {
            struct dirent *entry;
            char tmp[256];
            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_name[0] == '.') continue;
                snprintf(tmp, sizeof(tmp), "%s/%s", test_dir, entry->d_name);
                if (unlink(tmp) != 0) {
                    KMRAW("Failed to unlink %s\n", tmp);
                }
            }
            closedir(dir);
        }
        if (rmdir(test_dir) != 0) {
            KMRAW("Failed to remove old test directory, please clean manually\n");
            return NULL;
        }
    }
    ret = mkdir(test_dir, 0777);
    if (ret != 0) {
        KMRAW("mkdir %s failed\n", test_dir);
        return NULL;
    }
    KMRAW("Created directory: %s\n", test_dir);

    for (j = 0; j < BUFFER_SIZE; j++) {
        write_buf[j] = (uint8_t)(j & 0xFF);
    }

    for (i = 0; i < TEST_FILE_COUNT; i++) {
        KMRAW("\n--- Writing file %d/%d ---\n", i+1, TEST_FILE_COUNT);
        snprintf(file_path, sizeof(file_path), "%s/speed_test_%d.txt", test_dir, i+1);

        int fd = open(file_path, O_CREAT | O_WRONLY | O_TRUNC, 0666);
        if (fd < 0) {
            KMRAW("open %s for write failed\n", file_path);
            return NULL;
        }

        uint32_t total_written = 0;
        start = hrt_absolute_time();
        KMRAW("Writing %u bytes to %s ...\n", TEST_FILE_SIZE, file_path);

        while (total_written < TEST_FILE_SIZE) {
            uint32_t to_write = (TEST_FILE_SIZE - total_written) > BUFFER_SIZE ?
                                BUFFER_SIZE : (TEST_FILE_SIZE - total_written);
            ssize_t n = write(fd, write_buf, to_write);
            if (n != (ssize_t)to_write) {
                KMRAW("write error %d %d\n", n, to_write);
                close(fd);
                return NULL;
            }
            total_written += n;
        }
        end = hrt_absolute_time();
        uint64_t write_time = end - start;
        total_write_time += write_time;
        double speed_kb = (double)TEST_FILE_SIZE / (write_time / 1000000.0) / 1024.0;
        KMRAW("Write done in %.2f s, speed = %.2f KB/s\n",
            (double)write_time / 1000000.0, speed_kb);

        close(fd);
    }

    for (i = 0; i < TEST_FILE_COUNT; i++) {
        KMRAW("\n--- Reading file %d/%d ---\n", i+1, TEST_FILE_COUNT);
        snprintf(file_path, sizeof(file_path), "%s/speed_test_%d.txt", test_dir, i+1);

        int fd = open(file_path, O_RDONLY);
        if (fd < 0) {
            KMRAW("open %s for read failed\n", file_path);
            return NULL;
        }

        uint32_t total_read = 0;
        int error = 0;
        start = hrt_absolute_time();
        KMRAW("Reading and verifying %s ...\n", file_path);

        while (total_read < TEST_FILE_SIZE) {
            uint32_t to_read = (TEST_FILE_SIZE - total_read) > BUFFER_SIZE ?
                            BUFFER_SIZE : (TEST_FILE_SIZE - total_read);
            ssize_t n = read(fd, read_buf, to_read);
            if (n != (ssize_t)to_read) {
                KMRAW("read error\n");
                close(fd);
                return NULL;
            }
            /* 验证数据 */
            for (j = 0; j < n; j++) {
                if (read_buf[j] != write_buf[j % BUFFER_SIZE]) {
                    KMRAW("Data mismatch at offset %u (read=0x%02X, expected=0x%02X)\n",
                        total_read + j, read_buf[j], write_buf[j % BUFFER_SIZE]);
                    error = 1;
                    break;
                }
            }
            if (error) break;
            total_read += n;
        }
        end = hrt_absolute_time();
        uint64_t read_time = end - start;
        total_read_time += read_time;
        double speed_kb = (double)TEST_FILE_SIZE / (read_time / 1000000.0) / 1024.0;
        KMRAW("Read done in %.2f s, speed = %.2f KB/s\n",
                (double)read_time / 1000000.0, speed_kb);

        close(fd);

        if (error) {
            KMRAW("Verification FAILED!\n");
            return NULL;
        } else {
            KMRAW("Verification PASSED.\n");
        }

        struct stat st;
        if (stat(file_path, &st) == 0) {
            KMRAW("File size: %lld bytes, mode: 0%o\n", (long long)st.st_size, st.st_mode);
        }
    }

    KMRAW("\n=== Average Write Speed: %.2f KB/s ===\n",
           (double)TEST_FILE_SIZE * TEST_FILE_COUNT / (total_write_time / 1000000.0) / 1024.0);
    KMRAW("=== Average Read Speed:  %.2f KB/s ===\n",
           (double)TEST_FILE_SIZE * TEST_FILE_COUNT / (total_read_time / 1000000.0) / 1024.0);

    KMRAW("\nDeleting test files...\n");
    DIR *dir = opendir(test_dir);
    if (dir) {
        struct dirent *entry;
        char tmp[256];
        int deleted = 0;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            snprintf(tmp, sizeof(tmp), "%s/%s", test_dir, entry->d_name);
            if (unlink(tmp) == 0) {
                KMRAW("Deleted: %s\n", entry->d_name);
                deleted++;
            } else {
                KMRAW("Failed to delete %s\n", entry->d_name);
            }
        }
        closedir(dir);
        KMRAW("Deleted %d files.\n", deleted);
    }

    if (rmdir(test_dir) == 0) {
        KMRAW("Directory '%s' removed.\n", test_dir);
    } else {
        KMRAW("Failed to remove directory (not empty or other error)\n");
    }

    KMRAW("\n--- After test ---\n");
    ls(base_dir);
    print_fs_capacity(base_dir);

    KMRAW("\nVFS stress test completed.\n");
    return 0;
}


static void* vfs_cromfs_test_process(void *p);
int vfs_cromfs_test(int argc, char **argv)
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

    task_id = pthread_create(&id, &attr, &vfs_cromfs_test_process, NULL);

    return 0;
}

char cromfs_test_buf[1024+1];
void *vfs_cromfs_test_process(void *p)
{
    pthread_setname_np(pthread_self(), "usr_test");

    int ret = mount(NULL, "/etc", "cromfs", 0, NULL);
    if (ret < 0) {
        KMINFO("mount /fs/mmcsd failed\r\n");
    }

    KMRAW("=== Filesystem capacity ===\n");
    print_fs_capacity("/etc");

    KMRAW("=== Initial directory listing ===\n");
    ls("/etc");

    DIR *dir = opendir("/etc");
    if (dir == NULL) {
        KMRAW("opendir /etc failed\n");
        return NULL;
    }

    struct dirent *entry;
    char first_file[256] = {0};
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        snprintf(first_file, sizeof(first_file), "/etc/%s", entry->d_name);
        struct stat st;
        if (stat(first_file, &st) < 0) continue;

        if (S_ISREG(st.st_mode)) {
            break;
        }
    }
    closedir(dir);

    if (first_file[0] == '\0') {
        KMRAW("No regular file found in /etc\n");
        return NULL;
    }

    KMRAW("\n=== First regular file: %s ===\n", first_file);

    int fd = open(first_file, O_RDONLY);
    if (fd < 0) {
        KMRAW("open %s failed\n", first_file);
        return NULL;
    }

    ssize_t n = read(fd, cromfs_test_buf, sizeof(cromfs_test_buf) - 1);
    if (n < 0) {
        KMRAW("read %s failed\n", first_file);
        close(fd);
        return NULL;
    }

    cromfs_test_buf[n] = '\0';
    KMRAW("=== File content (first %d bytes) ===\n", (int)n);

    for (int i = 0; i < n; i++) {
        write(STDOUT_FILENO, &cromfs_test_buf[i], 1);
    }
    KMRAW("\n");

    close(fd);

    return NULL;
}
