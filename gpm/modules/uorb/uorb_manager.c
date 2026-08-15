#include <gpmx/config.h>

#include "uorb_manager.h"
#include "uorb_device_node.h"
#include "uorb_gnode.h"
#include <errno.h>
#include <driver/drv_sched.h>
#include <mlog.h>

#ifdef ORB_USE_PUBLISHER_RULES
static struct publisher_rule _publisher_rule;
static bool _has_publisher_rules = false;
static bool starts_with(const char *pre, const char *str);
static bool find_topic(const struct publisher_rule *rule, const char *topic_name);
static void strtrim(const char **str);
#endif

static struct __uorb_manager *_umanager_instance = NULL;
bool uorb_manager_initialize()
{
	if (_umanager_instance == NULL) {
		_umanager_instance = kmm_malloc(sizeof(struct __uorb_manager));
		if (_umanager_instance != NULL) {
			_umanager_instance->_device_master = NULL;
		}
	}

	return _umanager_instance != NULL;
}

bool uorb_manager_terminate()
{
	if (_umanager_instance != NULL) {
		uorb_device_master_deinit(_umanager_instance->_device_master);
		kmm_free(_umanager_instance->_device_master);
		kmm_free(_umanager_instance);

		_umanager_instance = NULL;
		return true;
	}

	return false;
}

struct __uorb_manager *uorb_manager_instance()
{
    return _umanager_instance;
}

struct __uorb_device_master *uorb_manager_get_device_master(struct __uorb_manager *manager)
{
    if (!manager->_device_master) {
        manager->_device_master = kmm_malloc(sizeof(struct __uorb_device_master));
		uorb_device_master_init(manager->_device_master);

        if (manager->_device_master == NULL) {
            // PX4_ERR("Failed to allocate DeviceMaster");
			errno = ENOMEM;
        }
    }

    return manager->_device_master;
}

static int manager_node_open(struct __uorb_manager *manager, const struct orb_metadata *meta, bool advertiser, 
	        int *instance, enum ORB_PRIO priority)
{
	char path[ORB_MAXPATH];
	int fd = -1;
	int ret = -1;

	/*
	 * If meta is null, the object was not defined, i.e. it is not
	 * known to the system.  We can't advertise/subscribe such a thing.
	 */
	if (NULL == meta) {
		return -1;
	}

	/* if we have an instance and are an advertiser, we will generate a new node and set the instance,
	 * so we do not need to open here */
	if (!instance || !advertiser) {
		/*
		 * Generate the path to the node and try to open it.
		 */
		ret = uorb_node_mkpath(path, meta, instance);
		if (ret != 0) {
			return -1;
		}

		/* open the path as either the advertiser or the subscriber */
		fd = unode_open(path, advertiser ? UORB_F_WRONLY : UORB_F_RDONLY);

		// KMDEBUG("fopen:%s %d \r\n", path, fd);
	} else {
		*instance = 0;
	}

	// KMDEBUG("handle instance%d \r\n", *instance);
	/* we may need to advertise the node... */
	if (fd < 0) {

		ret = -1;
		// KMDEBUG("op1\r\n");
		if (uorb_manager_get_device_master(manager)) {
			ret = uorb_device_master_advertise(manager->_device_master, meta, advertiser, instance, priority);
		}

		/* it's OK if it already exists */
		if ((ret != 0) && (EEXIST == errno)) {
			ret = 0;
		}

		// KMDEBUG("op2 %d %d\r\n", ret, errno);
		if (ret == 0) {
			/* update the path, as it might have been updated during the node advertise call */
			ret = uorb_node_mkpath(path, meta, instance);

			/* on success, try to open again */
			if (ret == 0) {
				fd = unode_open(path, (advertiser) ? UORB_F_WRONLY : UORB_F_RDONLY);

				// KMDEBUG("open:%d\r\n",fd);
			} else {
				errno = -ret;
				return -1;
			}
		}
	}

	if (fd < 0) {
		errno = EIO;
		return -1;
	}

	/* everything has been OK, we can return the handle now */
	return fd;
}

orb_advert_t orb_manager_advertise(struct __uorb_manager *manager, const struct orb_metadata *meta, const void *data, unsigned int queue_size)
{
    return orb_manager_advertise_multi(manager, meta, data, NULL, ORB_PRIO_DEFAULT, queue_size);
}

orb_advert_t orb_manager_advertise_multi(struct __uorb_manager *manager, const struct orb_metadata *meta, const void *data, int *instance, 
                    enum ORB_PRIO priority, unsigned int queue_size)
{

#ifdef ORB_USE_PUBLISHER_RULES

	// check publisher rule
	if (_has_publisher_rules) {

		extern const char *px_get_taskname(void);
		const char *prog_name = px_get_taskname();

		if (strcmp(_publisher_rule.module_name, prog_name) == 0) {
			if (_publisher_rule.ignore_other_topics) {
				if (!find_topic(&_publisher_rule, meta->o_name)) {
					// not allowing (prog_name) to publish topic (meta->o_name)
					return (orb_advert_t)_umanager_instance;
				}
			}
		} else {
			if (find_topic(&_publisher_rule, meta->o_name)) {
				// not allowing (prog_name) to publish topic (meta->o_name)
				return (orb_advert_t)_umanager_instance;
			}
		}
	}

#endif

	/* open the node as an advertiser */
	int fd = manager_node_open(manager, meta, true, instance, priority);

	// KMDEBUG("ad2 %d %u \r\n", fd, xPortGetFreeHeapSize());
	if (fd == -1) {
		KMERROR("%s advertise failed (%i)\r\n", meta->o_name, errno);
		return NULL;
	}

	/* Set the queue size. This must be done before the first publication; thus it fails if
	 * this is not the first advertiser.
	 */
	int result = unode_ioctl(fd, ORBIOCSETQUEUESIZE, (unsigned long)queue_size);

	// KMDEBUG("ad3 %u  set queue: %d \r\n", xPortGetFreeHeapSize(), result);
	if (result < 0 && queue_size > 1) {
		KMWARN("orb_advertise_multi: failed to set queue size\r\n");
	}

	/* get the advertiser handle and close the node */
	orb_advert_t advertiser;

	result = unode_ioctl(fd, ORBIOCGADVERTISER, (unsigned long)&advertiser);
	unode_close(fd);

	if (result == -1) {
		KMWARN("px4_ioctl ORBIOCGADVERTISER failed. fd = %d\r\n", fd);
		return NULL;
	}

	// KMDEBUG("ad4 %x %u \r\n", advertiser, xPortGetFreeHeapSize());

	/* the advertiser may perform an initial publish to initialise the object */
	if (data != NULL) {
		result = orb_manager_publish(manager, meta, advertiser, data);
		if (result == -1) {
			KMERROR("orb_publish failed %s %d\r\n", meta->o_name, errno);
			return NULL;
		}
	}

	// KMDEBUG("ad5 %u\r\n", xPortGetFreeHeapSize());
	return advertiser;
}

int orb_manager_unadvertise(struct __uorb_manager *manager, orb_advert_t handle)
{
#ifdef ORB_USE_PUBLISHER_RULES
	if (handle == _umanager_instance) {
		return 0; //pretend success
	}
#endif

    return uorb_device_node_unadvertise(handle);
}

int orb_manager_publish(struct __uorb_manager *manager, const struct orb_metadata *meta, orb_advert_t handle, const void *data)
{
#ifdef ORB_USE_PUBLISHER_RULES
	if (handle == _umanager_instance) {
		return 0; //pretend success
	}
#endif

    return uorb_device_node_publish(meta, handle, data);
}

int orb_manager_subscribe(struct __uorb_manager *manager, const struct orb_metadata *meta)
{
	return manager_node_open(manager, meta, false, NULL, ORB_PRIO_DEFAULT);
}

int orb_manager_subscribe_multi(struct __uorb_manager *manager, const struct orb_metadata *meta, unsigned instance)
{
	int inst = instance;
	return manager_node_open(manager, meta, false, &inst, ORB_PRIO_DEFAULT);
}

int orb_manager_unsubscribe(struct __uorb_manager *manager, int fd)
{
	return unode_close(fd);
}

int orb_manager_copy(struct __uorb_manager *manager, const struct orb_metadata *meta, int handle, void *buffer)
{
	int ret;

	ret = unode_read(handle, buffer, meta->o_size);

	if (ret < 0) {
		return -1;
	}

	if (ret != (int)meta->o_size) {
		return -1;
	}

	return 0;
}

int orb_manager_check(struct __uorb_manager *manager, int handle, bool *updated)
{
	/* Set to false here so that if `px4_ioctl` fails to false. */
	*updated = false;
	return unode_ioctl(handle, ORBIOCUPDATED, (unsigned long)(uintptr_t)updated);
}

int orb_manager_exists(struct __uorb_manager *manager, const struct orb_metadata *meta, int instance)
{
    int ret = -1;

    // instance valid range: [0, ORB_MULTI_MAX_INSTANCES)
    if ((instance < 0) || (instance > (ORB_MULTI_MAX_INSTANCES - 1))) {
        return ret;
    }

    if (uorb_manager_get_device_master(manager)) {
        struct __uorb_device_node *node = uorb_device_master_get_node2(manager->_device_master, meta, instance);

        if (node != NULL) {
            if (uorb_device_node_is_advertised(node)) {
                return 0;
            }
        }
    }

    return ret;
}

int orb_manager_priority(struct __uorb_manager *manager, int handle, enum ORB_PRIO *priority)
{
    return unode_ioctl(handle, ORBIOCGPRIORITY, (unsigned long)(uintptr_t)priority);
}

int orb_manager_set_interval(struct __uorb_manager *manager, int handle, unsigned interval)
{
    return unode_ioctl(handle, ORBIOCSETINTERVAL, interval * 1000);
}

int	orb_manager_get_interval(struct __uorb_manager *manager, int handle, unsigned *interval)
{
	int ret = unode_ioctl(handle, ORBIOCGETINTERVAL, (unsigned long)interval);
	*interval /= 1000;
	return ret;
}

#ifdef ORB_USE_PUBLISHER_RULES

bool starts_with(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre);
    size_t lenstr = strlen(str);

    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

bool find_topic(const struct publisher_rule *rule, const char *topic_name)
{
    const char **topics_ptr = rule->topics;

    while (*topics_ptr) {
        if (strcmp(*topics_ptr, topic_name) == 0) {
            return true;
        }

        ++topics_ptr;
    }

    return false;
}

void strtrim(const char **str)
{
    while (**str == ' ' || **str == '\t') { ++(*str); }
}

typedef struct {
    const char *cur;
    const char *end;
} mem_file;

static char *mem_fgets(char *buf, int size, mem_file *mf)
{
    if (!mf || mf->cur >= mf->end || size <= 0) {
        return NULL;
    }

    int i = 0;
    while (i < size - 1 && mf->cur < mf->end) {
        char c = *mf->cur++;
        buf[i++] = c;
        if (c == '\n') {
            break;
        }
    }
    buf[i] = '\0';
    return buf;
}

void publisher_rule_printl()
{
	publisher_rule_print(&_publisher_rule);
}

void publisher_rule_print(const struct publisher_rule *rule)
{
    if (!rule) {
        KMINFO("publisher_rule is NULL\n");
        return;
    }

    KMRAW("publisher_rule {\n");
    KMRAW("  module_name: %s\n", rule->module_name ? rule->module_name : "(none)");
    KMRAW("  ignore_other_topics: %s\n", rule->ignore_other_topics ? "true" : "false");
    KMRAW("  topics: ");
    if (rule->topics) {
        const char **t = rule->topics;
        int idx = 0;
        while (*t) {
            KMRAW("[%02d] %s ", idx++, *t);
            t++;
			if (*t) {
				KMRAW("\r\n          ");
			}
        }
        KMRAW("\n");
    } else {
        KMRAW("(null)\n");
    }
    KMRAW("}\n");
}

int read_publisher_rules_from_file(const char *file_content, ssize_t file_length, struct publisher_rule *rule)
{
	static const int line_len = 1024;
	int ret = 0;
	char *line = kmm_malloc(sizeof(char)*line_len);
	const char *restrict_topics_str = "restrict_topics:";
	const char *module_str = "module:";
	const char *ignore_others = "ignore_others:";

	if (!line || file_length <= 0 || !file_content) {
		return -ENOMEM;
	}

	mem_file mf;
	mf.cur = file_content;
	mf.end = file_content + file_length;

	rule->ignore_other_topics = false;
	rule->module_name = NULL;
	rule->topics = NULL;

	while (mem_fgets(line, line_len, &mf) && ret == 0) {

		if (strlen(line) < 2 || line[0] == '#') {
			continue;
		}

		if (starts_with(restrict_topics_str, line)) {
			//read topics list
			char *start = line + strlen(restrict_topics_str);
			strtrim((const char **)&start);
			char *topics = strdup(start);
			int topic_len = 0, num_topics = 0;

			for (int i = 0; topics[i]; ++i) {
				if (topics[i] == ',' || topics[i] == '\n') {
					if (topic_len > 0) {
						topics[i] = 0;
						++num_topics;
					}

					topic_len = 0;

				} else {
					++topic_len;
				}
			}

			if (num_topics > 0) {
				// rule.topics = new const char *[num_topics + 1];
				rule->topics = (const char **)kmm_malloc((num_topics + 1) * sizeof(const char *));
				int topic = 0;
				strtrim((const char **)&topics);
				rule->topics[topic++] = topics;

				while (topic < num_topics) {
					if (*topics == 0) {
						++topics;
						strtrim((const char **)&topics);
						rule->topics[topic++] = topics;

					} else {
						++topics;
					}
				}

				rule->topics[num_topics] = NULL;
			}

		} else if (starts_with(module_str, line)) {
			//read module name
			char *start = line + strlen(module_str);
			strtrim((const char **)&start);
			int len = strlen(start);

			if (len > 0 && start[len - 1] == '\n') {
				start[len - 1] = 0;
			}

			rule->module_name = strdup(start);

		} else if (starts_with(ignore_others, line)) {
			const char *start = line + strlen(ignore_others);
			strtrim(&start);

			if (starts_with("true", start)) {
				rule->ignore_other_topics = true;
			}

		} else {
			// orb rules file: wrong format
			ret = -EINVAL;
		}
	}

	if (ret == 0 && (!rule->module_name || !rule->topics)) {
		// Wrong format in orb publisher rules file
		ret = -EINVAL;
	}

	kmm_free(line);
	return ret;
}

int publisher_rule_config(const char *file_content, ssize_t file_length)
{
	int ret = read_publisher_rules_from_file(file_content, file_length, &_publisher_rule);
	if (0 == ret) {
		_has_publisher_rules = true;
		return 0;
	}

	_has_publisher_rules = false;
	return ret;
}
#endif
