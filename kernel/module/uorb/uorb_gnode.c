#include "uorb_gnode.h"
#include <pthread.h>
#include "kmodule_defines.h"
#include <semaphore.h>
#include <string.h>

#include "container/container_of.h"
#include "uorb_device_node.h"

#define NODEMAP_LENGTH  (128)

static struct urbnode *node_map[NODEMAP_LENGTH] = {0};
static pthread_mutex_t node_mutex = PTHREAD_MUTEX_INITIALIZER;

int register_driver(const char *name, void *data)
{
    int ret = -ENOSPC;

    struct __uorb_device_node *unode = NULL;
    unode = container_of(data, struct __uorb_device_node, nd);

    if (name == NULL || data == NULL) {
        return -EINVAL;
    }

    pthread_mutex_lock(&node_mutex);

    // Make sure the device does not already exist
    for (int i = 0; i < NODEMAP_LENGTH; i++) {
        if (node_map[i] && (strcmp(node_map[i]->_devname, name) == 0)) {
            pthread_mutex_unlock(&node_mutex);
            return -EEXIST;
        }
    }

    for (int i = 0; i < NODEMAP_LENGTH; i++) {
        if (node_map[i] == NULL) {
            node_map[i] = (struct urbnode *)data;
            KMDEBUG("Registered DEV %d %x %x %s\r\n", i, node_map[i], unode, name);
            ret = 0;
            break;
        }
    }

    pthread_mutex_unlock(&node_mutex);

    if (ret != 0) {
        // KMERROR("No free nodemap entries - increase nodemap size");
    }

    return ret;
}

int unregister_driver(const char *name)
{
    int ret = -EINVAL;
    if (name == NULL) {
        return -EINVAL;
    }

    pthread_mutex_lock(&node_mutex);

    for (int i = 0; i < NODEMAP_LENGTH; i++) {
        if (node_map[i] && (strcmp(node_map[i]->_devname, name) == 0)) {
            node_map[i] = NULL;
            KMDEBUG("Unregistered DEV %s\r\n", name);
            ret = 0;
            break;
        }
    }

    pthread_mutex_unlock(&node_mutex);

    return ret;
}

struct urbnode *get_uorbnode(int fd)
{
    pthread_mutex_lock(&node_mutex);
    struct urbnode *dev = NULL;

    if (fd < NODEMAP_LENGTH && fd >= 0) {
        dev = node_map[fd];
    }

    pthread_mutex_unlock(&node_mutex);
    return dev;
}

struct urbnode *get_uorbnode2(const char *name)
{
	pthread_mutex_lock(&node_mutex);

    for (int i = 0; i < NODEMAP_LENGTH; i++) {
        if (node_map[i] && (strcmp(node_map[i]->_devname, name) == 0)) {
            pthread_mutex_unlock(&node_mutex);
            return node_map[i];
        }
    }

	pthread_mutex_unlock(&node_mutex);

	return NULL;
}

int get_uorbnodefd(const char *name)
{
    int i = 0;
	pthread_mutex_lock(&node_mutex);

    for (i = 0; i < NODEMAP_LENGTH; i++) {
        if (node_map[i] && (strcmp(node_map[i]->_devname, name) == 0)) {
            pthread_mutex_unlock(&node_mutex);
            return i;
        }
    }

	pthread_mutex_unlock(&node_mutex);

	return i;
}

/****************************************************************************
 * UORBNODE
 ****************************************************************************/
void uorbnode_init(struct urbnode *node, const char *devname)
{
    node->_priv = NULL;
    node->_oflags = 0;
    node->_registered = false;
    node->_devname = devname;
    sem_init(&node->_lock, 0, 1);
}

void uorbnode_deinit(struct urbnode *node)
{
    if (node->_registered) {
        unregister_driver(node->_devname);
    }

    sem_destroy(&node->_lock);
}

void urbnode_lock(struct urbnode *node)
{
    do {} while (sem_wait(&node->_lock) != 0);
}

void urbnode_unlock(struct urbnode *node)
{
    sem_post(&node->_lock);
}

int uorbnode_register(struct urbnode *node)
{
	int ret = 0;

	// now register the driver
	if (node->_devname != NULL) {
		ret = register_driver(node->_devname, (void *)node);

		if (ret == 0) {
			node->_registered = true;
		}
	}

	return ret;
}

const char *uorbnode_name(struct urbnode *node)
{
    return node->_devname;
}

int unode_open(const char *path, int flags)
{
    int ret = 0;
    int i = get_uorbnodefd(path);
    struct urbnode *dev = NULL;

    if (i == NODEMAP_LENGTH) {
        return -1;
    }

    dev = get_uorbnode(i);

	if (dev) {
        dev->_oflags = flags;
		ret = dev->open(dev);

	} else {
		ret = -EINVAL;
	}

	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return i;
}

int unode_close(int fd)
{
    int ret;
    struct urbnode *dev = get_uorbnode(fd);

    if (dev) {
        ret = dev->close(dev);

    } else {
        ret = -EINVAL;
    }

    if (ret < 0) {
        ret = -1;
    }

    return ret;
}

ssize_t unode_read(int fd, void *buffer, size_t buflen)
{
    int ret;
    struct urbnode *dev = get_uorbnode(fd);

    if (dev) {
        ret = dev->read(dev, (char *)buffer, buflen);
    } else {
        ret = -EINVAL;
    }

    if (ret < 0) {
        ret = -1;
    }

    return ret;
}

ssize_t unode_write(int fd, const void *buffer, size_t buflen)
{
    int ret;
    struct urbnode *dev = get_uorbnode(fd);

    if (dev) {
        ret = dev->write(dev, (char *)buffer, buflen);
    } else {
        ret = -EINVAL;
    }

    if (ret < 0) {
        ret = -1;
    }

    return ret;
}

int unode_ioctl(int fd, int cmd, unsigned long arg)
{
    int ret = 0;
    struct urbnode *dev = get_uorbnode(fd);

    if (dev) {
        ret = dev->ioctl(dev, cmd, arg);

    } else {
        ret = -EINVAL;
    }

    return ret;
}

