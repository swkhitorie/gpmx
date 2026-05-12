#ifndef PUBLICATION_MULTI_CPP_H_
#define PUBLICATION_MULTI_CPP_H_

#include "uorb_common.h"
#include "Publication.hpp"

namespace uORB
{

/**
 * Base publication multi wrapper class
 */
template<typename T, uint8_t QSIZE = 1>
class PublicationMulti : public PublicationBase
{
public:

	/**
	 * Constructor
	 *
	 * @param meta The uORB metadata (usually from the ORB_ID() macro) for the topic.
	 * @param priority The priority for multi pub/sub, 0 means don't publish as multi
	 */
	PublicationMulti(enum ORB_ID id, ORB_PRIO priority = ORB_PRIO_DEFAULT) :
		PublicationBase(id),
		_priority(priority)
	{}

	PublicationMulti(const orb_metadata *meta, ORB_PRIO priority = ORB_PRIO_DEFAULT) :
		PublicationBase(static_cast<ORB_ID>(meta->o_id)),
		_priority(priority)
	{}

	bool advertise()
	{
		if (!advertised()) {
			int instance = 0;
            _handle = orb_advertise_multi_queue(get_topic(), NULL, &instance, _priority, QSIZE);
		}

		return advertised();
	}

	/**
	 * Publish the struct
	 * @param data The uORB message struct we are updating.
	 */
	bool publish(const T &data)
	{
		if (!advertised()) {
			advertise();
		}

		return (orb_publish(get_topic(), _handle, &data) == PX4_OK);
	}

	int get_instance()
	{
		// advertise if not already advertised
		if (advertise()) {
			return uorb_device_node_get_instance(static_cast<struct __uorb_device_node *>(_handle));
			// return Manager::orb_get_instance(_handle);
		}

		return -1;
	}

protected:
	const ORB_PRIO _priority;
};

/**
 * The publication multi class with data embedded.
 */
template<typename T>
class PublicationMultiData : public PublicationMulti<T>
{
public:
	/**
	 * Constructor
	 *
	 * @param meta The uORB metadata (usually from the ORB_ID() macro) for the topic.
	 * @param priority The priority for multi pub
	 */
	PublicationMultiData(enum ORB_ID id, ORB_PRIO priority = ORB_PRIO_DEFAULT) :
		PublicationMulti<T>(id, priority)
	{}
	PublicationMultiData(const orb_metadata *meta, ORB_PRIO priority = ORB_PRIO_DEFAULT) :
		PublicationMulti<T>(meta, priority)
	{}

	T	&get() { return _data; }
	void	set(const T &data) { _data = data; }

	// Publishes the embedded struct.
	bool	update() { return PublicationMulti<T>::publish(_data); }
	bool	update(const T &data)
	{
		_data = data;
		return PublicationMulti<T>::publish(_data);
	}

private:
	T _data{};
};

template<class T>
using PublicationQueuedMulti = PublicationMulti<T, T::ORB_QUEUE_LENGTH>;

} // namespace uORB

#endif

