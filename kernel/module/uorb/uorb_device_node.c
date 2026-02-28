#include "uorb_device_node.h"
#include "subscription_callback.h"
#include "uorb_gnode.h"
#include "kmodule_defines.h"

#include "FreeRTOS.h"

struct __update_interval_data {
    uint64_t last_update; /**< time at which the last update was provided, used when update_interval is nonzero */
    unsigned interval; /**< if nonzero minimum interval between updates */
};

struct __subscriber_data {
    uint32_t generation; /**< last generation the subscriber has seen */
    struct __update_interval_data *update_interval; /**< if null, no update interval */
};

bool uorb_device_cmp_method(struct intrusive_node *a, struct intrusive_node *b)
{
    struct __uorb_device_node *uorb_node_a;
    struct __uorb_device_node *uorb_node_b;
    uorb_node_a = container_of(a, struct __uorb_device_node, sorted_list_node);
    uorb_node_b = container_of(b, struct __uorb_device_node, sorted_list_node);

    return (strcmp(uorbnode_name(&uorb_node_a->nd), uorbnode_name(&uorb_node_b->nd)) <= 0);
}

static bool uorb_device_node_copy_locked(struct __uorb_device_node *unode, void *dst, uint32_t *generation)
{
	bool updated = false;

	if ((dst != NULL) && (unode->_data != NULL)) {
		const unsigned int current_generation = atomic_uint32_load(&unode->_generation);

		if (current_generation > (*generation) + unode->_queue_size) {
			// Reader is too far behind: some messages are lost
			*generation = current_generation - unode->_queue_size;
		}

		if ((current_generation == *generation) && (*generation > 0)) {
			/* The subscriber already read the latest message, but nothing new was published yet.
			 * Return the previous message
			 */
			(*generation)--;
		}

		memcpy(dst, unode->_data + (unode->_meta->o_size * ((*generation) % unode->_queue_size)), unode->_meta->o_size);
		if (*generation < current_generation) {
			(*generation)++;
		}

		updated = true;
	}

	return updated;
}

static bool uorb_device_node_appears_updated(struct __uorb_device_node *unode)
{
	// check if this topic has been published yet, if not bail out
	if (unode->_data == NULL) {
		return false;
	}

	struct __subscriber_data *sd = (struct __subscriber_data *)(unode->nd._priv);

	// if subscriber has interval check time since last update
	if (sd->update_interval != NULL) {
		if (hrt_elapsed_time(&sd->update_interval->last_update) < sd->update_interval->interval) {
			return false;
		}
	}

	// finally, compare the generation
	return (sd->generation != uorb_device_node_published_message_count(unode));
}

void uorb_device_node_init(struct __uorb_device_node *unode, struct orb_metadata *meta, const uint8_t instance, 
            const char *path, enum ORB_PRIO priority, uint8_t queue_size)
{
	uorbnode_init(&unode->nd, path);
    unode->nd.open = uorb_device_node_open;
	unode->nd.close = uorb_device_node_close;
	unode->nd.read = uorb_device_node_read;
	unode->nd.write = uorb_device_node_write;
	unode->nd.ioctl = uorb_device_node_ioctl;

	unode->_meta = meta;
	unode->_priority = priority;
	unode->_instance = instance;
	unode->_queue_size = queue_size;
	unode->_data = NULL;

	intrusive_node_init(&unode->sorted_list_node);
	intrusive_list_init(&unode->_callbacks);
	atomic_uint32_init(&unode->_generation, 0);
}

void uorb_device_node_deinit(struct __uorb_device_node *unode)
{
	uorbnode_deinit(&unode->nd);
	vPortFree(unode->_data);
	free((void *)unode->nd._devname);
}

int uorb_device_node_open(struct urbnode *node)
{
    struct __uorb_device_node *unode = NULL;
    unode = container_of(node, struct __uorb_device_node, nd);

	KMDEBUG("node_open %d \r\n", unode->nd._oflags);
	/* is this a publisher? */
	if (unode->nd._oflags == UORB_F_WRONLY) {

		urbnode_lock(&unode->nd);
		uorb_device_node_mark_as_advertised(unode);
		urbnode_unlock(&unode->nd);

		/* now complete the open */
		return 0;
	}

	/* is this a new subscriber? */
	if (unode->nd._oflags == UORB_F_RDONLY) {

		/* allocate subscriber data */
		struct __subscriber_data *sd = pvPortMalloc(sizeof(struct __subscriber_data));
		sd->update_interval = NULL;
		if (NULL == sd) {
			return -ENOMEM;
		}

		/* If there were any previous publications, allow the subscriber to read them */
		const unsigned gen = uorb_device_node_published_message_count(unode);
		sd->generation = gen - (unode->_queue_size < gen ? unode->_queue_size : gen);

		unode->nd._priv = (void *)sd;

		uorb_device_node_add_internal_subscriber(unode);

		return 0;
	}

	/* can only be pub or sub, not both */
	return -EINVAL;
}

int uorb_device_node_close(struct urbnode *node)
{
    struct __uorb_device_node *unode = NULL;
    unode = container_of(node, struct __uorb_device_node, nd);

	if (unode->nd._oflags == UORB_F_RDONLY) { 
		/* subscriber */
		struct __subscriber_data *sd = (struct __subscriber_data *)(unode->nd._priv);

		if (sd != NULL) {
			uorb_device_node_remove_internal_subscriber(unode);

			if (sd->update_interval) {
				vPortFree(sd->update_interval);
			}
			vPortFree(sd);
			sd = NULL;
		}
	}

	return 0;
}

ssize_t uorb_device_node_read(struct urbnode *node, char *buffer, size_t buflen)
{
    struct __uorb_device_node *unode = NULL;
    unode = container_of(node, struct __uorb_device_node, nd);

	/* if the object has not been written yet, return zero */
	if (unode->_data == NULL) {
		return 0;
	}

	/* if the caller's buffer is the wrong size, that's an error */
	if (buflen != unode->_meta->o_size) {
		return -EIO;
	}

	struct __subscriber_data *sd = (struct __subscriber_data *)(unode->nd._priv);

	/*
	 * Perform an atomic copy & state update
	 */
	ATOMIC_ENTER;

	// if subscriber has an interval track the last update time
	if (sd->update_interval) {
		sd->update_interval->last_update = hrt_absolute_time();
	}

	uorb_device_node_copy_locked(unode, buffer, &sd->generation);

	ATOMIC_LEAVE;

	return unode->_meta->o_size;
}

ssize_t uorb_device_node_write(struct urbnode *node, const char *buffer, size_t buflen)
{
    struct __uorb_device_node *unode = NULL;
    struct intrusive_node *iterator = NULL;
    struct __subscription_callback *item = NULL;
    uint32_t generation = 0;

    unode = container_of(node, struct __uorb_device_node, nd);

	/*
	 * Writes are legal from interrupt context as long as the
	 * object has already been initialised from thread context.
	 *
	 * Writes outside interrupt context will allocate the object
	 * if it has not yet been allocated.
	 *
	 * Note that filp will usually be NULL.
	 */
	if (NULL == unode->_data) {

		if (xPortIsInsideInterrupt() != pdTRUE) {
			urbnode_lock(&unode->nd);

			/* re-check size */
			if (NULL == unode->_data) {
				unode->_data = pvPortMalloc(unode->_meta->o_size * unode->_queue_size);
			}

			urbnode_unlock(&unode->nd);
		}


		/* failed or could not allocate */
		if (NULL == unode->_data) {
			return -ENOMEM;
		}
	}

	/* If write size does not match, that is an error */
	if (unode->_meta->o_size != buflen) {
		return -EIO;
	}

	/* Perform an atomic copy. */
	ATOMIC_ENTER;

	/* wrap-around happens after ~49 days, assuming a publisher rate of 1 kHz */
	generation = atomic_uint32_fetch_add(&unode->_generation, 1);

	memcpy(unode->_data + (unode->_meta->o_size * (generation % unode->_queue_size)), buffer, unode->_meta->o_size);

    // callbacks
    for (iterator = unode->_callbacks.head; iterator != NULL; iterator = iterator->next) {
        item = container_of(iterator, struct __subscription_callback, list_node);

		item->call(item);
    }

	ATOMIC_LEAVE;

	/* notify any poll waiters */
	// poll_notify(POLLIN);

	return unode->_meta->o_size;
}

int uorb_device_node_ioctl(struct urbnode *node, int cmd, unsigned long arg)
{
    struct __uorb_device_node *unode = NULL;
    unode = container_of(node, struct __uorb_device_node, nd);

	switch (cmd) {
	case ORBIOCUPDATED: {
			ATOMIC_ENTER;
			*(bool *)arg = uorb_device_node_appears_updated(unode);
			ATOMIC_LEAVE;
			return 0;
		}

	case ORBIOCSETINTERVAL: {
			int ret = 0;
			urbnode_lock(&unode->nd);

			struct __subscriber_data *sd = (struct __subscriber_data *)(unode->nd._priv);

			if (arg == 0) {
				if (sd->update_interval) {
					vPortFree(sd->update_interval);
					sd->update_interval = NULL;
				}

			} else {
				if (sd->update_interval) {
					sd->update_interval->interval = arg;

				} else {
					sd->update_interval = pvPortMalloc(sizeof(struct __update_interval_data));

					if (sd->update_interval) {
						sd->update_interval->interval = arg;

					} else {
						ret = -12;
					}
				}
			}

			urbnode_unlock(&unode->nd);
			return ret;
		}

	case ORBIOCGADVERTISER:
		*(uintptr_t *)arg = (uintptr_t)unode;
		return 0;

	case ORBIOCGPRIORITY:
		*(int *)arg = uorb_device_node_get_priority(unode);
		return 0;

	case ORBIOCSETQUEUESIZE: {
			urbnode_lock(&unode->nd);
			int ret = uorb_device_node_update_queue_size(unode, arg);
			urbnode_unlock(&unode->nd);
			return ret;
		}

	case ORBIOCGETINTERVAL: {
		    struct __subscriber_data *sd = (struct __subscriber_data *)(unode->nd._priv);

			if (sd->update_interval) {
				*(unsigned *)arg = sd->update_interval->interval;

			} else {
				*(unsigned *)arg = 0;
			}
		}

		return 0;

	case ORBIOCISADVERTISED:
		*(unsigned long *)arg = unode->_advertised;
		return 0;

	default:
		return 0;
	}
}

ssize_t uorb_device_node_publish(const struct orb_metadata *meta, orb_advert_t handle, const void *data)
{
	struct __uorb_device_node *devnode = (struct __uorb_device_node *)handle;
	int ret;

	/* check if the device handle is initialized and data is valid */
	if ((devnode == NULL) || (meta == NULL) || (data == NULL)) {
		errno = EFAULT;
		return -1;
	}

	/* check if the orb meta data matches the publication */
	if (devnode->_meta != meta) {
		errno = EINVAL;
		return -1;
	}

	/* call the devnode write method with no file pointer */
	ret = uorb_device_node_write(&devnode->nd, (const char *)data, meta->o_size);

	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	if (ret != (int)meta->o_size) {
		errno = EIO;
		return -1;
	}

	return 0;
}

int uorb_device_node_unadvertise(orb_advert_t handle)
{
	if (handle == NULL) {
		return -EINVAL;
	}

	struct __uorb_device_node *devnode = (struct __uorb_device_node *)handle;

	/*
	 * We are cheating a bit here. First, with the current implementation, we can only
	 * have multiple publishers for instance 0. In this case the caller will have
	 * instance=nullptr and _published has no effect at all. Thus no unadvertise is
	 * necessary.
	 * In case of multiple instances, we have at most 1 publisher per instance and
	 * we can signal an instance as 'free' by setting _published to false.
	 * We never really free the DeviceNode, for this we would need reference counting
	 * of subscribers and publishers. But we also do not have a leak since future
	 * publishers reuse the same DeviceNode object.
	 */
	devnode->_advertised = false;

	KMDEBUG("unadvertise: %s\r\n", devnode->nd._devname);
	return 0;
}

void uorb_device_node_add_internal_subscriber(struct __uorb_device_node *unode)
{
	urbnode_lock(&unode->nd);
	unode->_subscriber_count++;
	urbnode_unlock(&unode->nd);
}

void uorb_device_node_remove_internal_subscriber(struct __uorb_device_node *unode)
{
	urbnode_lock(&unode->nd);
	unode->_subscriber_count--;
	urbnode_unlock(&unode->nd);
}

/**
 * Return true if this topic has been advertised.
 *
 * This is used in the case of multi_pub/sub to check if it's valid to advertise
 * and publish to this node or if another node should be tried.
 */
bool uorb_device_node_is_advertised(struct __uorb_device_node *unode)
{ 
    return unode->_advertised; 
}

void uorb_device_node_mark_as_advertised(struct __uorb_device_node *unode)
{ 
    unode->_advertised = true; 
}

uint8_t uorb_device_node_get_queue_size(struct __uorb_device_node *unode)
{ 
    return unode->_queue_size; 
}

int8_t uorb_device_node_subscriber_count(struct __uorb_device_node *unode)
{ 
    return unode->_subscriber_count; 
}

uint32_t uorb_device_node_published_message_count(struct __uorb_device_node *unode)
{ 
    return atomic_uint32_load(&unode->_generation);
}

enum ORB_ID uorb_device_node_id(struct __uorb_device_node *unode)
{ 
    return (enum ORB_ID)(unode->_meta->o_id); 
}

const char *uorb_device_node_get_name(struct __uorb_device_node *unode)
{ 
    return unode->_meta->o_name; 
}

uint8_t uorb_device_node_get_instance(struct __uorb_device_node *unode)
{ 
    return unode->_instance; 
}

enum ORB_PRIO uorb_device_node_get_priority(struct __uorb_device_node *unode)
{ 
    return (enum ORB_PRIO)unode->_priority; 
}

void uorb_device_node_set_priority(struct __uorb_device_node *unode, enum ORB_PRIO priority)
{ 
    unode->_priority = priority; 
}

struct orb_metadata *uorb_device_node_get_meta(struct __uorb_device_node *unode)
{ 
    return unode->_meta; 
}

bool uorb_device_node_print_statistics(struct __uorb_device_node *unode, int max_topic_length)
{
	if (!unode->_advertised) {
		return false;
	}

	urbnode_lock(&unode->nd);

    const struct orb_metadata *meta = uorb_device_node_get_meta(unode);
    const uint8_t instance = uorb_device_node_get_instance(unode);
    const uint8_t priority = uorb_device_node_get_priority(unode);
    const int8_t sub_count = uorb_device_node_subscriber_count(unode);
    const uint8_t queue_size = uorb_device_node_get_queue_size(unode);

	urbnode_unlock(&unode->nd);

	KMINFO("%-*s %2i %4i %2i %4i %4i %s\n", max_topic_length, meta->o_name, (int)instance, (int)sub_count,
        queue_size, meta->o_size, priority, uorbnode_name(&unode->nd));

	return true;
}

int uorb_device_node_update_queue_size(struct __uorb_device_node *unode, unsigned int queue_size)
{
	if (unode->_queue_size == queue_size) {
		return 0;
	}

	// queue size is limited to 255 for the single reason that we use uint8 to store it
	if (unode->_data || unode->_queue_size > queue_size || queue_size > 255) {
		return -1;
	}

	unode->_queue_size = queue_size;
	return 0;
}

bool uorb_device_node_copy(struct __uorb_device_node *unode, void *dst, uint32_t *generation)
{
	ATOMIC_ENTER;

	bool updated = uorb_device_node_copy_locked(unode, dst, generation);

	ATOMIC_LEAVE;

	return updated;
}

bool uorb_device_node_register_callback(struct __uorb_device_node *unode, struct __subscription_callback *callback_sub)
{
    struct intrusive_node *iterator = NULL;
    struct __subscription_callback *existing_callbacks;

	if (callback_sub != NULL) {
		ATOMIC_ENTER;

        // prevent duplicate registrations
        for (iterator = unode->_callbacks.head; iterator != NULL; iterator = iterator->next) {
            existing_callbacks = container_of(iterator, struct __subscription_callback, list_node);

            if (callback_sub == existing_callbacks) {
                ATOMIC_LEAVE;
                return true;
            }
        }

		intrusive_list_add(&unode->_callbacks, &callback_sub->list_node);

		ATOMIC_LEAVE;
		return true;
	}

	return false;
}

void uorb_device_node_unregister_callback(struct __uorb_device_node *unode, struct __subscription_callback *callback_sub)
{
	ATOMIC_ENTER;
	intrusive_list_remove(&unode->_callbacks, &callback_sub->list_node);
	ATOMIC_LEAVE;
}

