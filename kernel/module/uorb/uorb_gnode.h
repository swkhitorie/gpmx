#ifndef UORB_GNODE_H_
#define UORB_GNODE_H_

#include <stdbool.h>
#include <stddef.h>
#include <semaphore.h>

struct urbnode {
    const char *_devname;
    bool _registered;

    sem_t _lock;
    int _oflags;
    void *_priv;

    int (*open)(struct urbnode *node);
    int (*close)(struct urbnode *node);
    ssize_t (*read)(struct urbnode *node, char *buffer, size_t buflen);
    ssize_t	(*write)(struct urbnode *node, const char *buffer, size_t buflen);
    int	(*ioctl)(struct urbnode *node, int cmd, unsigned long arg);
};

#ifdef __cplusplus
extern "C" {
#endif

void uorbnode_init(struct urbnode *node, const char *devname);
void uorbnode_deinit(struct urbnode *node);
void urbnode_lock(struct urbnode *node);
void urbnode_unlock(struct urbnode *node);
int  uorbnode_register(struct urbnode *node);
const char *uorbnode_name(struct urbnode *node);

int unode_open(const char *path, int flags);
int unode_close(int fd);
ssize_t unode_read(int fd, void *buffer, size_t buflen);
ssize_t unode_write(int fd, const void *buffer, size_t buflen);
int unode_ioctl(int fd, int cmd, unsigned long arg);


#ifdef __cplusplus
}
#endif

#endif
