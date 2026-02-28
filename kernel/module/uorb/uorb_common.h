#ifndef _UORB_UORB_H
#define _UORB_UORB_H

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

#include <drv_hrt.h>

/**
 * Topic priority.
 * Relevant for multi-topics / topic groups
 */
enum ORB_PRIO {
    ORB_PRIO_UNINITIALIZED = 0,
    ORB_PRIO_MIN = 1,
    ORB_PRIO_VERY_LOW = 25,
    ORB_PRIO_LOW = 50,
    ORB_PRIO_DEFAULT = 75,
    ORB_PRIO_HIGH = 100,
    ORB_PRIO_VERY_HIGH = 125,
    ORB_PRIO_MAX = 255
};

struct orb_metadata {
	const char *o_name;		            /**< unique object name */
	const uint16_t o_size;		        /**< object size */
	const uint16_t o_size_no_padding;	/**< object size w/o padding at the end (for logger) */
	const char *o_fields;		        /**< semicolon separated list of fields (with type) */
	uint8_t o_id;			            /**< ORB_ID enum */
};

typedef struct orb_metadata *orb_id_t;

struct orb_advertdata {
    struct orb_metadata *meta;
    int *instance;
    enum ORB_PRIO priority;
};

#define ATOMIC_ENTER uint32_t flags = portSET_INTERRUPT_MASK_FROM_ISR()
#define ATOMIC_LEAVE portCLEAR_INTERRUPT_MASK_FROM_ISR(flags)

#define UORB_F_RDONLY 1
#define UORB_F_WRONLY 2

/**
 * Maximum number of multi topic instances
 */
#define ORB_MULTI_MAX_INSTANCES	4 // This must be < 10 (because it's the last char of the node path)

#define ORB_MAXPATH 64

/*
 * IOCTLs for individual topics.
 */

#define _UORBIOC(_n)		((0x2600)|(_n))

/** Check whether the topic has been updated since it was last read, sets *(bool *)arg */
#define ORBIOCUPDATED		_UORBIOC(11)

/** Set the minimum interval at which the topic can be seen to be updated for this subscription */
#define ORBIOCSETINTERVAL	_UORBIOC(12)

/** Get the global advertiser handle for the topic */
#define ORBIOCGADVERTISER	_UORBIOC(13)

/** Get the priority for the topic */
#define ORBIOCGPRIORITY		_UORBIOC(14)

/** Set the queue size of the topic */
#define ORBIOCSETQUEUESIZE	_UORBIOC(15)

/** Get the minimum interval at which the topic can be seen to be updated for this subscription */
#define ORBIOCGETINTERVAL	_UORBIOC(16)

/** Check whether the topic is advertised, sets *(unsigned long *)arg to 1 if advertised, 0 otherwise */
#define ORBIOCISADVERTISED	_UORBIOC(17)

/**
 * Generates a pointer to the uORB metadata structure for
 * a given topic.
 *
 * The topic must have been declared previously in scope
 * with ORB_DECLARE().
 *
 * @param _name		The name of the topic.
 */
#define ORB_ID(_name)     &__orb_##_name

/**
 * Declare (prototype) the uORB metadata for a topic (used by code generators).
 *
 * @param _name		The name of the topic.
 */
#if defined(__cplusplus)
#define ORB_DECLARE(_name)    extern "C" struct orb_metadata __orb_##_name __EXPORT
#else
#define ORB_DECLARE(_name)    extern struct orb_metadata __orb_##_name __EXPORT
#endif

/**
 * Define (instantiate) the uORB metadata for a topic.
 *
 * The uORB metadata is used to help ensure that updates and
 * copies are accessing the right data.
 *
 * Note that there must be no more than one instance of this macro
 * for each topic.
 *
 * @param _name		The name of the topic.
 * @param _struct	The structure the topic provides.
 * @param _size_no_padding	Struct size w/o padding at the end
 * @param _fields	All fields in a semicolon separated list e.g: "float[3] position;bool armed"
 * @param _orb_id_enum	ORB ID enum e.g.: ORB_ID::vehicle_status
 */
#define ORB_DEFINE(_name, _struct, _size_no_padding, _fields, _orb_id_enum)		\
	struct orb_metadata __orb_##_name = {	\
		#_name,					\
		sizeof(_struct),		\
		_size_no_padding,			\
		_fields,				\
		_orb_id_enum				\
	}; struct hack

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ORB topic advertiser handle.
 *
 * Advertiser handles are global; once obtained they can be shared freely
 * and do not need to be closed or released.
 *
 * This permits publication from interrupt context and other contexts where
 * a file-descriptor-based handle would not otherwise be in scope for the
 * publisher.
 */
typedef void 	*orb_advert_t;

extern orb_advert_t orb_advertise(struct orb_metadata *meta, const void *data);

extern orb_advert_t orb_advertise_queue(struct orb_metadata *meta, const void *data,
					unsigned int queue_size);

extern orb_advert_t orb_advertise_multi(struct orb_metadata *meta, const void *data, int *instance,
					enum ORB_PRIO priority);

extern orb_advert_t orb_advertise_multi_queue(struct orb_metadata *meta, const void *data, int *instance,
		enum ORB_PRIO priority, unsigned int queue_size);

extern int orb_unadvertise(orb_advert_t handle);

extern int	orb_publish(struct orb_metadata *meta, orb_advert_t handle, const void *data);

/**
 * Advertise as the publisher of a topic.
 *
 * This performs the initial advertisement of a topic; it creates the topic
 * node in /obj if required and publishes the initial data.
 */
static inline int orb_publish_auto(struct orb_metadata *meta, orb_advert_t *handle, const void *data,
				   int *instance, enum ORB_PRIO priority)
{
	if (!*handle) {
		*handle = orb_advertise_multi(meta, data, instance, priority);

		if (*handle) {
			return 0;
		}

	} else {
		return orb_publish(meta, *handle, data);
	}

	return -1;
}

extern int	orb_subscribe(struct orb_metadata *meta);

extern int	orb_subscribe_multi(struct orb_metadata *meta, unsigned instance);

extern int	orb_unsubscribe(int handle);

extern int  orb_copy(struct orb_metadata *meta, int handle, void *buffer);

extern int  orb_check(int handle, bool *updated);

extern int	orb_exists(struct orb_metadata *meta, int instance);

/**
 * Get the number of published instances of a topic group
 */
extern int	orb_group_count(struct orb_metadata *meta);

extern int	orb_priority(int handle, enum ORB_PRIO *priority);

extern int	orb_set_interval(int handle, unsigned interval);

extern int	orb_get_interval(int handle, unsigned *interval);

extern int uorb_node_mkpath(char *buf, struct orb_metadata *meta, int *instance);

extern int uorb_node_mkpath2(char *buf, const char *orbMsgName);

#ifdef __cplusplus
}
#endif

#endif
