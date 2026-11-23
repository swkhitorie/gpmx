/****************************************************************************
 *
 *   Copyright (c) 2012-2022 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file parameters.cpp
 *
 * Global parameter store.
 *
 * Note that it might make sense to convert this into a driver.  That would
 * offer some interesting options regarding state for e.g. ORB advertisements
 * and background parameter saving.
 */

#define PARAM_IMPLEMENTATION
#include "param.h"
#include "param_translation.h"
#include <parameters/px4_parameters.hpp>
#include <lib/tinybson/tinybson.h>

#include <float.h>
#include <math.h>
#include <errno.h>
#include <stdint.h>

#include <containers/Bitset.hpp>

#include "perf/perf_counter.h"

#include <platform/common/log.h>
#include <platform/common/atomic_bitset.h>
#include <platform/common/defines.h>
#include <platform/common/posix.h>
#include "uthash/utarray.h"

#include <drivers/drv_hrt.h>
using namespace time_literals;

#include "uorb/uorb_common.h"
#include "uorb/topics/parameter_update.h"
#include <uorb/topics/actuator_armed.h>
#include <uorb/Subscription.hpp>

/* Include functions common to user and kernel sides */
#include "parameters_common.cpp"

#if defined(FLASH_BASED_PARAMS)
#include "flashparams/flashparams.h"
#else
inline static int flash_param_save(param_filter_func filter) { return -1; }
inline static int flash_param_load() { return -1; }
inline static int flash_param_import() { return -1; }
#endif

static char *param_default_file = nullptr;
static char *param_backup_file = nullptr;

#include "wqueue/wqueue.h"

/* autosaving variables */
static uint64_t last_autosave_timestamp = 0;
static struct work_s autosave_work {};
static px4::atomic_bool autosave_scheduled{false};
static bool autosave_disabled = false;

static px4::AtomicBitset<param_info_count> params_active;  // params found
static px4::AtomicBitset<param_info_count> params_changed; // params non-default
static px4::Bitset<param_info_count> params_custom_default; // params with runtime default value
static px4::AtomicBitset<param_info_count> params_unsaved;

// Storage for modified parameters.
struct param_wbuf_s {
	union param_value_u val;
	param_t             param;
};

/** flexible array holding modified parameter values */
UT_array *param_values{nullptr};
UT_array *param_custom_default_values{nullptr};

const UT_icd param_icd = {sizeof(param_wbuf_s), nullptr, nullptr, nullptr};

/** parameter update topic handle */
static orb_advert_t param_topic = nullptr;
static unsigned int param_instance = 0;

// the following implements an RW-lock using 2 semaphores (used as mutexes). It gives
// priority to readers, meaning a writer could suffer from starvation, but in our use-case
// we only have short periods of reads and writes are rare.
static sem_t param_sem; ///< this protects against concurrent access to param_values
static int reader_lock_holders = 0;
static sem_t reader_lock_holders_lock; ///< this protects against concurrent access to reader_lock_holders

static perf_counter_t param_export_perf;
static perf_counter_t param_find_perf;
static perf_counter_t param_get_perf;
static perf_counter_t param_set_perf;

static sem_t param_sem_save; ///< this protects against concurrent param saves (file or flash access).
///< we use a separate lock to allow concurrent param reads and saves.
///< a param_set could still be blocked by a param save, because it
///< needs to take the reader lock

static const uint32_t crc32_tab[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

static uint32_t crc32part(const uint8_t *src, size_t len, uint32_t crc32val)
{
	size_t i;

	for (i = 0;  i < len;  i++) {
		crc32val = crc32_tab[(crc32val ^ src[i]) & 0xff] ^ (crc32val >> 8);
	}

	return crc32val;
}

static uint32_t crc32(const uint8_t *src, size_t len)
{
	return crc32part(src, len, 0);
}

/** lock the parameter store for read access */
static void
param_lock_reader()
{
	do {} while (sem_wait(&reader_lock_holders_lock) != 0);

	++reader_lock_holders;

	if (reader_lock_holders == 1) {
		// the first reader takes the lock, the next ones are allowed to just continue
		do {} while (sem_wait(&param_sem) != 0);
	}

	sem_post(&reader_lock_holders_lock);
}

/** lock the parameter store for write access */
static void
param_lock_writer()
{
	do {} while (sem_wait(&param_sem) != 0);
}

/** unlock the parameter store */
static void
param_unlock_reader()
{
	do {} while (sem_wait(&reader_lock_holders_lock) != 0);

	--reader_lock_holders;

	if (reader_lock_holders == 0) {
		// the last reader releases the lock
		sem_post(&param_sem);
	}

	sem_post(&reader_lock_holders_lock);
}

/** unlock the parameter store */
static void
param_unlock_writer()
{
	sem_post(&param_sem);
}

/** assert that the parameter store is locked */
static void
param_assert_locked()
{
	/* XXX */
}

void
param_init()
{
	sem_init(&param_sem, 0, 1);
	sem_init(&param_sem_save, 0, 1);
	sem_init(&reader_lock_holders_lock, 0, 1);

	param_export_perf = perf_alloc(PC_ELAPSED, "param: export");
	param_find_perf = perf_alloc(PC_COUNT, "param: find");
	param_get_perf = perf_alloc(PC_COUNT, "param: get");
	param_set_perf = perf_alloc(PC_ELAPSED, "param: set");

}

/**
 * Compare two modified parameter structures to determine ordering.
 *
 * This function is suitable for passing to qsort or bsearch.
 */
static int
param_compare_values(const void *a, const void *b)
{
	struct param_wbuf_s *pa = (struct param_wbuf_s *)a;
	struct param_wbuf_s *pb = (struct param_wbuf_s *)b;

	if (pa->param < pb->param) {
		return -1;
	}

	if (pa->param > pb->param) {
		return 1;
	}

	return 0;
}

/**
 * Locate the modified parameter structure for a parameter, if it exists.
 *
 * @param param			The parameter being searched.
 * @return			The structure holding the modified value, or
 *				nullptr if the parameter has not been modified.
 */
static param_wbuf_s *
param_find_changed(param_t param)
{
	param_assert_locked();

	if (params_changed[param] && (param_values != nullptr)) {
		param_wbuf_s key{};
		key.param = param;
		return (param_wbuf_s *)utarray_find(param_values, &key, param_compare_values);
	}

	return nullptr;
}

void
param_notify_changes()
{
	parameter_update_s pup{};
	pup.instance = param_instance++;
	pup.get_count = perf_event_count(param_get_perf);
	pup.set_count = perf_event_count(param_set_perf);
	pup.find_count = perf_event_count(param_find_perf);
	pup.export_count = perf_event_count(param_export_perf);
	pup.active = params_active.count();
	pup.changed = params_changed.count();
	pup.custom_default = params_custom_default.count();
	pup.timestamp = hrt_absolute_time();

	if (param_topic == nullptr) {
		param_topic = orb_advertise(ORB_ID(parameter_update), &pup);

	} else {
		orb_publish(ORB_ID(parameter_update), param_topic, &pup);
	}
}
#include <platform/common/log.h>
static param_t param_find_internal(const char *name, bool notification)
{
	perf_count(param_find_perf);
	param_t middle;
	param_t front = 0;
	param_t last = param_info_count;

	/* perform a binary search of the known parameters */
	while (front <= last) {
		middle = front + (last - front) / 2;
		int ret = strcmp(name, param_name(middle));

		if (ret == 0) {
			if (notification) {
				param_set_used(middle);
			}

			return middle;

		} else if (middle == front) {
			/* An end point has been hit, but there has been no match */
			break;

		} else if (ret < 0) {
			last = middle;

		} else {
			front = middle;
		}
	}

	// PX4_ERR("param %s not find\r\n", name);

	/* not found */
	return PARAM_INVALID;
}

param_t param_find(const char *name)
{
	return param_find_internal(name, true);
}

param_t param_find_no_notification(const char *name)
{
	return param_find_internal(name, false);
}

unsigned param_count_used()
{
	return params_active.count();
}

param_t param_for_used_index(unsigned index)
{
	// walk all params and count used params
	if (index < param_info_count) {
		unsigned used_count = 0;

		for (int i = 0; i < params_active.size(); i++) {
			if (params_active[i]) {
				// we found the right used count,
				//  return the param value
				if (index == used_count) {
					return static_cast<param_t>(i);
				}

				used_count++;
			}
		}
	}

	return PARAM_INVALID;
}

int param_get_used_index(param_t param)
{
	/* this tests for out of bounds and does a constant time lookup */
	if (!param_used(param)) {
		return -1;
	}

	/* walk all params and count, now knowing that it has a valid index */
	int used_count = 0;

	for (int i = 0; i < params_active.size(); i++) {
		if (params_active[i]) {

			if (param == i) {
				return used_count;
			}

			used_count++;
		}
	}

	return -1;
}

bool
param_value_unsaved(param_t param)
{
	return handle_in_range(param) ? params_unsaved[param] : false;
}

/**
 * Obtain a pointer to the storage allocated for a parameter.
 *
 * @param param			The parameter whose storage is sought.
 * @return			A pointer to the parameter value, or nullptr
 *				if the parameter does not exist.
 */
static const void *
param_get_value_ptr(param_t param)
{
	param_assert_locked();

	if (handle_in_range(param)) {
		/* work out whether we're fetching the default or a written value */
		struct param_wbuf_s *s = param_find_changed(param);

		if (s != nullptr) {
			return &s->val;

		} else {
			if (params_custom_default[param] && param_custom_default_values) {
				// get default from custom default storage
				param_wbuf_s key{};
				key.param = param;
				param_wbuf_s *pbuf = (param_wbuf_s *)utarray_find(param_custom_default_values, &key, param_compare_values);

				if (pbuf != nullptr) {
					return &pbuf->val;
				}
			}

			// otherwise return static default value
			switch (param_type(param)) {
			case PARAM_TYPE_INT32:
				return &px4::parameters[param].val.i;

			case PARAM_TYPE_FLOAT:
				return &px4::parameters[param].val.f;
			}
		}
	}

	return nullptr;
}

int
param_get(param_t param, void *val)
{
	perf_count(param_get_perf);

	if (!handle_in_range(param)) {
		PX4_ERR("get: param %" PRId16 " invalid", param);
		return PX4_ERROR;
	}

	if (!params_active[param]) {
		PX4_DEBUG("get: param %" PRId16 " (%s) not active", param, param_name(param));
	}

	int result = PX4_ERROR;

	if (val) {
		if (!params_changed[param] && !params_custom_default[param]) {
			// if parameter is unchanged (static default value) copy immediately and avoid locking
			switch (param_type(param)) {
			case PARAM_TYPE_INT32:
				memcpy(val, &px4::parameters[param].val.i, sizeof(px4::parameters[param].val.i));
				return PX4_OK;

			case PARAM_TYPE_FLOAT:
				memcpy(val, &px4::parameters[param].val.f, sizeof(px4::parameters[param].val.f));
				return PX4_OK;
			}
		}

		param_lock_reader();
		const void *v = param_get_value_ptr(param);

		if (v) {
			memcpy(val, v, param_size(param));
			result = PX4_OK;
		}

		param_unlock_reader();
	}

	return result;
}

int
param_get_default_value_internal(param_t param, void *default_val)
{
	if (!handle_in_range(param)) {
		PX4_ERR("get default value: param %d invalid", param);
		return PX4_ERROR;
	}

	if (default_val) {
		if (params_custom_default[param] && param_custom_default_values) {
			// get default from custom default storage
			param_wbuf_s key{};
			key.param = param;
			param_wbuf_s *pbuf = (param_wbuf_s *)utarray_find(param_custom_default_values, &key, param_compare_values);

			if (pbuf != nullptr) {
				memcpy(default_val, &pbuf->val, param_size(param));
				return PX4_OK;
			}
		}

		// otherwise return static default value
		switch (param_type(param)) {
		case PARAM_TYPE_INT32:
			memcpy(default_val, &px4::parameters[param].val.i, param_size(param));
			return PX4_OK;

		case PARAM_TYPE_FLOAT:
			memcpy(default_val, &px4::parameters[param].val.f, param_size(param));
			return PX4_OK;
		}
	}

	return PX4_ERROR;
}

int
param_get_default_value(param_t param, void *default_val)
{
	if (!handle_in_range(param)) {
		return PX4_ERROR;
	}

	int ret = 0;

	if (!params_custom_default[param]) {
		// return static default value
		switch (param_type(param)) {
		case PARAM_TYPE_INT32:
			memcpy(default_val, &px4::parameters[param].val.i, sizeof(px4::parameters[param].val.i));
			return PX4_OK;

		case PARAM_TYPE_FLOAT:
			memcpy(default_val, &px4::parameters[param].val.f, sizeof(px4::parameters[param].val.f));
			return PX4_OK;
		}

	} else {
		param_lock_reader();
		ret = param_get_default_value_internal(param, default_val);
		param_unlock_reader();
	}

	return ret;
}

bool param_value_is_default(param_t param)
{
	if (!handle_in_range(param)) {
		return true;
	}

	if (!params_changed[param] && !params_custom_default[param]) {
		// no value saved and no custom default
		return true;

	} else {
		// the param_values dynamic array might carry things that have been set
		// back to default, so we don't rely on the params_changed bitset here
		switch (param_type(param)) {
		case PARAM_TYPE_INT32: {
				param_lock_reader();
				int32_t default_value = 0;

				if (param_get_default_value_internal(param, &default_value) == PX4_OK) {
					const void *v = param_get_value_ptr(param);

					if (v) {
						bool unchanged = (*static_cast<const int32_t *>(v) == default_value);
						param_unlock_reader();
						return unchanged;
					}
				}

				param_unlock_reader();
			}
			break;

		case PARAM_TYPE_FLOAT: {
				param_lock_reader();
				float default_value = 0;

				if (param_get_default_value_internal(param, &default_value) == PX4_OK) {
					const void *v = param_get_value_ptr(param);

					if (v) {
						bool unchanged = (fabsf(*static_cast<const float *>(v) - default_value) <= FLT_EPSILON);
						param_unlock_reader();
						return unchanged;
					}
				}

				param_unlock_reader();
			}
			break;
		}
	}

	return true;
}

/**
 * worker callback method to save the parameters
 * @param arg unused
 */
static void
autosave_worker(void *arg)
{
	bool disabled = false;

	if (!param_get_default_file()) {
		// In case we save to FLASH, defer param writes until disarmed,
		// as writing to FLASH can stall the entire CPU (in rare cases around 300ms on STM32F7)
		uORB::SubscriptionData<actuator_armed_s> armed_sub{ORB_ID(actuator_armed)};

		if (armed_sub.get().armed) {
			work_queue(LPWORK, &autosave_work, (worker_t)&autosave_worker, nullptr, USEC2TICK(1_s));
			return;
		}
	}

	param_lock_writer();
	last_autosave_timestamp = hrt_absolute_time();
	autosave_scheduled.store(false);
	disabled = autosave_disabled;
	param_unlock_writer();

	if (disabled) {
		return;
	}

	PX4_DEBUG("Autosaving params");
	int ret = param_save_default();

	if (ret != 0) {
		PX4_ERR("param auto save failed (%i)", ret);
	}
}

/**
 * Automatically save the parameters after a timeout and limited rate.
 *
 * This needs to be called with the writer lock held (it's not necessary that it's the writer lock, but it
 * needs to be the same lock as autosave_worker() and param_control_autosave() use).
 */
static void
param_autosave()
{
	if (autosave_scheduled.load() || autosave_disabled) {
		return;
	}

	// wait at least 300ms before saving, because:
	// - tasks often call param_set() for multiple params, so this avoids unnecessary save calls
	// - the logger stores changed params. He gets notified on a param change via uORB and then
	//   looks at all unsaved params.
	hrt_abstime delay = 300_ms;

	static constexpr const hrt_abstime rate_limit = 2_s; // rate-limit saving to 2 seconds
	const hrt_abstime last_save_elapsed = hrt_elapsed_time(&last_autosave_timestamp);

	if (last_save_elapsed < rate_limit && rate_limit > last_save_elapsed + delay) {
		delay = rate_limit - last_save_elapsed;
	}

	autosave_scheduled.store(true);
	work_queue(LPWORK, &autosave_work, (worker_t)&autosave_worker, nullptr, USEC2TICK(delay));
}

void
param_control_autosave(bool enable)
{
	param_lock_writer();

	if (!enable && autosave_scheduled.load()) {
		work_cancel(LPWORK, &autosave_work);
		autosave_scheduled.store(false);
	}

	autosave_disabled = !enable;
	param_unlock_writer();
}

static int
param_set_internal(param_t param, const void *val, bool mark_saved, bool notify_changes)
{
	if (!handle_in_range(param)) {
		PX4_ERR("set invalid param %d", param);
		return PX4_ERROR;
	}

	if (val == nullptr) {
		PX4_ERR("set invalid value");
		return PX4_ERROR;
	}

	int result = -1;
	bool param_changed = false;

	param_lock_writer();
	perf_begin(param_set_perf);

	// create the parameter store if it doesn't exist
	if (param_values == nullptr) {
		utarray_new(param_values, &param_icd);
		// mark all parameters unchanged (default)
		params_changed.reset();
		params_unsaved.reset();
	}

	if (param_values == nullptr) {
		PX4_ERR("failed to allocate modified values array");
		goto out;

	} else {
		param_wbuf_s *s = param_find_changed(param);

		if (s == nullptr) {
			/* construct a new parameter */
			param_wbuf_s buf{};
			buf.param = param;

			param_changed = true;

			/* add it to the array and sort */
			utarray_push_back(param_values, &buf);
			utarray_sort(param_values, param_compare_values);
			params_changed.set(param, true);

			/* find it after sorting */
			s = param_find_changed(param);
		}

		if (s == nullptr) {
			PX4_ERR("error param_values storage slot invalid");

		} else {
			/* update the changed value */
			switch (param_type(param)) {
			case PARAM_TYPE_INT32:
				if (s->val.i != *(int32_t *)val) {
					s->val.i = *(int32_t *)val;
					param_changed = true;
				}

				params_changed.set(param, true);
				params_unsaved.set(param, !mark_saved);
				result = PX4_OK;
				break;

			case PARAM_TYPE_FLOAT:
				if (fabsf(s->val.f - * (float *)val) > FLT_EPSILON) {
					s->val.f = *(float *)val;
					param_changed = true;
				}

				params_changed.set(param, true);
				params_unsaved.set(param, !mark_saved);
				result = PX4_OK;
				break;

			default:
				PX4_ERR("param_set invalid param type for %s", param_name(param));
				break;
			}
		}

		if ((result == PX4_OK) && param_changed && !mark_saved) { // this is false when importing parameters
			param_autosave();
		}
	}

out:
	perf_end(param_set_perf);
	param_unlock_writer();

	/*
	 * If we set something, now that we have unlocked, go ahead and advertise that
	 * a thing has been set.
	 */
	if ((result == PX4_OK) && param_changed && notify_changes) {
		param_notify_changes();
	}

	return result;
}

#if defined(FLASH_BASED_PARAMS)
int param_set_external(param_t param, const void *val, bool mark_saved, bool notify_changes)
{
	return param_set_internal(param, val, mark_saved, notify_changes);
}

const void *param_get_value_ptr_external(param_t param)
{
	return param_get_value_ptr(param);
}
#endif

int param_set(param_t param, const void *val)
{
	return param_set_internal(param, val, false, true);
}

int param_set_no_notification(param_t param, const void *val)
{
	return param_set_internal(param, val, false, false);
}

bool param_used(param_t param)
{
	if (handle_in_range(param)) {
		return params_active[param];
	}

	return false;
}

void param_set_used(param_t param)
{
	if (handle_in_range(param)) {
		params_active.set(param, true);
	}
}

int param_set_default_value(param_t param, const void *val)
{
	if (!handle_in_range(param)) {
		PX4_ERR("set default value invalid param %d", param);
		return PX4_ERROR;
	}

	if (val == nullptr) {
		PX4_ERR("set default value invalid value");
		return PX4_ERROR;
	}

	int result = PX4_ERROR;

	param_lock_writer();

	if (param_custom_default_values == nullptr) {
		utarray_new(param_custom_default_values, &param_icd);

		// mark all parameters unchanged (default)
		params_custom_default.reset();

		if (param_custom_default_values == nullptr) {
			PX4_ERR("failed to allocate custom default values array");
			param_unlock_writer();
			return PX4_ERROR;
		}
	}

	// check if param being set to default value
	bool setting_to_static_default = false;

	switch (param_type(param)) {
	case PARAM_TYPE_INT32:
		setting_to_static_default = (px4::parameters[param].val.i == *(int32_t *)val);
		break;

	case PARAM_TYPE_FLOAT:
		setting_to_static_default = (fabsf(px4::parameters[param].val.f - * (float *)val) <= FLT_EPSILON);
		break;
	}

	// find if custom default value is already set
	param_wbuf_s *s = nullptr;

	{
		param_wbuf_s key{};
		key.param = param;
		s = (param_wbuf_s *)utarray_find(param_custom_default_values, &key, param_compare_values);
	}

	if (setting_to_static_default) {
		if (s != nullptr) {
			// param in memory and set to non-default value, clear
			int pos = utarray_eltidx(param_custom_default_values, s);
			utarray_erase(param_custom_default_values, pos, 1);
		}

		// do nothing if param not already set and being set to default
		params_custom_default.set(param, false);
		result = PX4_OK;

	} else {
		if (s == nullptr) {
			// construct a new parameter default value
			param_wbuf_s buf{};
			buf.param = param;

			// add it to the array and sort
			utarray_push_back(param_custom_default_values, &buf);
			utarray_sort(param_custom_default_values, param_compare_values);

			// find it after sorting
			s = (param_wbuf_s *)utarray_find(param_custom_default_values, &buf, param_compare_values);
		}

		if (s != nullptr) {
			// update the default value
			switch (param_type(param)) {
			case PARAM_TYPE_INT32:
				s->val.i = *(int32_t *)val;
				params_custom_default.set(param, true);
				result = PX4_OK;
				break;

			case PARAM_TYPE_FLOAT:
				s->val.f = *(float *)val;
				params_custom_default.set(param, true);
				result = PX4_OK;
				break;

			default:
				break;
			}
		}
	}

	param_unlock_writer();

	if ((result == PX4_OK) && param_used(param)) {
		// send notification if param is already in use
		param_notify_changes();
	}

	return result;
}

static int param_reset_internal(param_t param, bool notify = true)
{
	param_wbuf_s *s = nullptr;
	bool param_found = false;

	param_lock_writer();

	if (handle_in_range(param)) {
		/* look for a saved value */
		s = param_find_changed(param);

		/* if we found one, erase it */
		if (s != nullptr) {
			int pos = utarray_eltidx(param_values, s);
			utarray_erase(param_values, pos, 1);
		}

		params_changed.set(param, false);
		params_unsaved.set(param, true);

		param_found = true;
	}

	param_autosave();

	param_unlock_writer();

	if (s != nullptr && notify) {
		param_notify_changes();
	}

	return (!param_found);
}

int param_reset(param_t param) { return param_reset_internal(param, true); }
int param_reset_no_notification(param_t param) { return param_reset_internal(param, false); }

static void
param_reset_all_internal(bool auto_save)
{
	param_lock_writer();

	if (param_values != nullptr) {
		utarray_free(param_values);

		params_changed.reset();
	}

	/* mark as reset / deleted */
	param_values = nullptr;

	if (auto_save) {
		param_autosave();
	}

	param_unlock_writer();

	param_notify_changes();
}

void
param_reset_all()
{
	param_reset_all_internal(true);
}

void
param_reset_excludes(const char *excludes[], int num_excludes)
{
	for (param_t param = 0; handle_in_range(param); param++) {
		const char *name = param_name(param);
		bool exclude = false;

		for (int index = 0; index < num_excludes; index ++) {
			int len = strlen(excludes[index]);

			if ((excludes[index][len - 1] == '*'
			     && strncmp(name, excludes[index], len - 1) == 0)
			    || strcmp(name, excludes[index]) == 0) {

				exclude = true;
				break;
			}
		}

		if (!exclude) {
			param_reset(param);
		}
	}
}

void
param_reset_specific(const char *resets[], int num_resets)
{
	for (param_t param = 0; handle_in_range(param); param++) {
		const char *name = param_name(param);
		bool reset = false;

		for (int index = 0; index < num_resets; index++) {
			int len = strlen(resets[index]);

			if ((resets[index][len - 1] == '*'
			     && strncmp(name, resets[index], len - 1) == 0)
			    || strcmp(name, resets[index]) == 0) {

				reset = true;
				break;
			}
		}

		if (reset) {
			param_reset(param);
		}
	}
}

int
param_set_default_file(const char *filename)
{
	if ((param_backup_file && strcmp(filename, param_backup_file) == 0)) {
		PX4_ERR("default file can't be the same as the backup file %s", filename);
		return PX4_ERROR;
	}

#ifdef FLASH_BASED_PARAMS
	// the default for flash-based params is always the FLASH
	(void)filename;
#else

	if (param_default_file != nullptr) {
		// we assume this is not in use by some other thread
		free(param_default_file);
		param_default_file = nullptr;
	}

	if (filename) {
		param_default_file = strdup(filename);
	}

#endif /* FLASH_BASED_PARAMS */

	return 0;
}

const char *param_get_default_file()
{
	return param_default_file;
}

int param_set_backup_file(const char *filename)
{
	if (param_default_file && strcmp(filename, param_default_file) == 0) {
		PX4_ERR("backup file can't be the same as the default file %s", filename);
		return PX4_ERROR;
	}

	if (param_backup_file != nullptr) {
		// we assume this is not in use by some other thread
		free(param_backup_file);
		param_backup_file = nullptr;
	}

	if (filename) {
		param_backup_file = strdup(filename);

	} else {
		param_backup_file = nullptr; // backup disabled
	}

	return 0;
}

const char *param_get_backup_file()
{
	return param_backup_file;
}

static int param_export_internal(int fd, param_filter_func filter);
static int param_verify(int fd);

int param_save_default()
{
	PX4_DEBUG("param_save_default");
	// int shutdown_lock_ret = px4_shutdown_lock();

	// if (shutdown_lock_ret != 0) {
	// 	PX4_ERR("px4_shutdown_lock() failed (%i)", shutdown_lock_ret);
	// }

	// take the file lock
	do {} while (sem_wait(&param_sem_save) != 0);

	param_lock_reader();

	int res = PX4_ERROR;
	const char *filename = param_get_default_file();

	if (filename) {
		static constexpr int MAX_ATTEMPTS = 3;

		for (int attempt = 1; attempt <= MAX_ATTEMPTS; attempt++) {
			// write parameters to file
			int fd = ::open(filename, O_WRONLY | O_CREAT | O_TRUNC, PX4_O_MODE_666);

			if (fd > -1) {
				perf_begin(param_export_perf);
				res = param_export_internal(fd, nullptr);
				perf_end(param_export_perf);
				::close(fd);

				if (res == PX4_OK) {
					// reopen file to verify
					int fd_verify = ::open(filename, O_RDONLY, PX4_O_MODE_666);
					res = param_verify(fd_verify) || lseek(fd_verify, 0, SEEK_SET) || param_verify(fd_verify);
					::close(fd_verify);
				}
			}

			if (res == PX4_OK) {
				break;

			} else {
				PX4_ERR("parameter export to %s failed (%d) attempt %d", filename, res, attempt);
				usleep(10000); // wait at least 10 milliseconds before trying again
			}
		}

	} else {
		perf_begin(param_export_perf);
		res = flash_param_save(nullptr);
		perf_end(param_export_perf);
	}

	if (res != PX4_OK) {
		PX4_ERR("param export failed (%d)", res);

	} else {
		params_unsaved.reset();

		// backup file
		if (param_backup_file) {
			int fd_backup_file = ::open(param_backup_file, O_WRONLY | O_CREAT | O_TRUNC, PX4_O_MODE_666);

			if (fd_backup_file > -1) {
				int backup_export_ret = param_export_internal(fd_backup_file, nullptr);
				::close(fd_backup_file);

				if (backup_export_ret != 0) {
					PX4_ERR("backup parameter export to %s failed (%d)", param_backup_file, backup_export_ret);

				} else {
					// verify export
					int fd_verify = ::open(param_backup_file, O_RDONLY, PX4_O_MODE_666);
					param_verify(fd_verify);
					::close(fd_verify);
				}
			}
		}
	}

	param_unlock_reader();
	sem_post(&param_sem_save);

	// if (shutdown_lock_ret == 0) {
	// 	px4_shutdown_unlock();
	// }

	return res;
}

/**
 * @return 0 on success, 1 if all params have not yet been stored, -1 if device open failed, -2 if writing parameters failed
 */
int
param_load_default()
{
	int res = 0;
	const char *filename = param_get_default_file();

	if (!filename) {
		return flash_param_load();
	}

	int fd_load = ::open(filename, O_RDONLY);

	if (fd_load < 0) {
		/* no parameter file is OK, otherwise this is an error */
		if (errno != ENOENT) {
			PX4_ERR("open '%s' for reading failed", filename);
			return -1;
		}

		return 1;
	}

	int result = param_load(fd_load);
	::close(fd_load);

	if (result != 0) {
		PX4_ERR("error reading parameters from '%s'", filename);
		return -2;
	}

	return res;
}

static int param_verify_callback(bson_decoder_t decoder, bson_node_t node)
{
	if (node->type == BSON_EOO) {
		return 0;
	}

	// find the parameter this node represents
	param_t param = param_find_no_notification(node->name);

	if (param == PARAM_INVALID) {
		PX4_ERR("verify: invalid parameter '%s'", node->name);
		return -1;
	}

	// handle verifying the parameter from the node
	switch (node->type) {
	case BSON_INT32: {
			if (param_type(param) != PARAM_TYPE_INT32) {
				PX4_ERR("verify: invalid param type %d for '%s' (BSON_INT32)", param_type(param), node->name);
				return -1;
			}

			int32_t value;

			if (param_get(param, &value) == 0) {
				if (value == node->i32) {
					return 1; // valid

				} else {
					PX4_ERR("verify: '%s' invalid BSON value %" PRIi32 "(expected %" PRIi32 ")", node->name, node->i32, value);
				}
			}
		}
		break;

	case BSON_DOUBLE: {
			if (param_type(param) != PARAM_TYPE_FLOAT) {
				PX4_ERR("verify: invalid param type %d for '%s' (BSON_DOUBLE)", param_type(param), node->name);
				return -1;
			}

			float value;

			if (param_get(param, &value) == 0) {
				if (fabsf(value - (float)node->d) <= FLT_EPSILON) {
					return 1; // valid

				} else {
					PX4_ERR("verify: '%s' invalid BSON value %.3f (expected %.3f)", node->name, node->d, (double)value);
				}
			}
		}
		break;

	default:
		PX4_ERR("verify: '%s' invalid node type %d", node->name, node->type);
	}

	return -1;
}

static int param_verify(int fd)
{
	PX4_DEBUG("param_verify");

	if (fd < 0) {
		return -1;
	}

	if (lseek(fd, 0, SEEK_SET) != 0) {
		PX4_ERR("verify: seek failed");
		return -1;
	}

	bson_decoder_s decoder{};

	if (bson_decoder_init_file(&decoder, fd, param_verify_callback) == 0) {
		int result = -1;

		do {
			result = bson_decoder_next(&decoder);

		} while (result > 0);

		if (result == 0) {
			if (decoder.total_document_size != decoder.total_decoded_size) {
				PX4_ERR("BSON document size (%" PRId32 ") doesn't match bytes decoded (%" PRId32 ")", decoder.total_document_size,
					decoder.total_decoded_size);

			} else {
				return 0;
			}

		} else if (result == -ENODATA) {
			PX4_ERR("verify: no BSON data");

		} else {
			PX4_ERR("verify: failed (%d)", result);
		}
	}

	return -1;
}

int
param_export(const char *filename, param_filter_func filter)
{
	PX4_DEBUG("param_export");

	// int shutdown_lock_ret = px4_shutdown_lock();

	// if (shutdown_lock_ret != 0) {
	// 	PX4_ERR("px4_shutdown_lock() failed (%i)", shutdown_lock_ret);
	// }

	// take the file lock
	do {} while (sem_wait(&param_sem_save) != 0);

	param_lock_reader();

	int fd = ::open(filename, O_RDWR | O_CREAT, PX4_O_MODE_666);
	int result = PX4_ERROR;

	perf_begin(param_export_perf);

	if (fd > -1) {
		result = param_export_internal(fd, filter);

	} else {
		result = flash_param_save(filter);
	}

	perf_end(param_export_perf);

	param_unlock_reader();
	sem_post(&param_sem_save);

	// if (shutdown_lock_ret == 0) {
	// 	px4_shutdown_unlock();
	// }

	return result;
}

// internal parameter export, caller is responsible for locking
static int param_export_internal(int fd, param_filter_func filter)
{
	PX4_DEBUG("param_export_internal");

	int result = -1;
	param_wbuf_s *s = nullptr;
	bson_encoder_s encoder{};
	uint8_t bson_buffer[256];

	if (lseek(fd, 0, SEEK_SET) != 0) {
		PX4_ERR("export seek failed %d", errno);
		return -1;
	}

	if (bson_encoder_init_buf_file(&encoder, fd, &bson_buffer, sizeof(bson_buffer)) != 0) {
		goto out;
	}

	// no modified parameters, export empty BSON document
	if (param_values == nullptr) {
		result = 0;
		goto out;
	}

	while ((s = (struct param_wbuf_s *)utarray_next(param_values, s)) != nullptr) {
		if (filter && !filter(s->param)) {
			continue;
		}

		// don't export default values
		switch (param_type(s->param)) {
		case PARAM_TYPE_INT32: {
				int32_t default_value = 0;
				param_get_default_value_internal(s->param, &default_value);

				if (s->val.i == default_value) {
					PX4_DEBUG("skipping %s %" PRIi32 " export", param_name(s->param), default_value);
					continue;
				}
			}
			break;

		case PARAM_TYPE_FLOAT: {
				float default_value = 0;
				param_get_default_value_internal(s->param, &default_value);

				if (fabsf(s->val.f - default_value) <= FLT_EPSILON) {
					PX4_DEBUG("skipping %s %.3f export", param_name(s->param), (double)default_value);
					continue;
				}
			}
			break;
		}

		const char *name = param_name(s->param);
		const size_t size = param_size(s->param);

		/* append the appropriate BSON type object */
		switch (param_type(s->param)) {
		case PARAM_TYPE_INT32: {
				const int32_t i = s->val.i;
				PX4_DEBUG("exporting: %s (%d) size: %lu val: %" PRIi32, name, s->param, (long unsigned int)size, i);

				if (bson_encoder_append_int32(&encoder, name, i) != 0) {
					PX4_ERR("BSON append failed for '%s'", name);
					goto out;
				}
			}
			break;

		case PARAM_TYPE_FLOAT: {
				const double f = (double)s->val.f;
				PX4_DEBUG("exporting: %s (%d) size: %lu val: %.3f", name, s->param, (long unsigned int)size, (double)f);

				if (bson_encoder_append_double(&encoder, name, f) != 0) {
					PX4_ERR("BSON append failed for '%s'", name);
					goto out;
				}
			}
			break;

		default:
			PX4_ERR("%s unrecognized parameter type %d, skipping export", name, param_type(s->param));
		}
	}

	result = 0;

out:

	if (result == 0) {
		if (bson_encoder_fini(&encoder) != PX4_OK) {
			PX4_ERR("BSON encoder finalize failed");
			result = -1;
		}
	}

	return result;
}

static int
param_import_callback(bson_decoder_t decoder, bson_node_t node)
{
	/*
	 * EOO means the end of the parameter object. (Currently not supporting
	 * nested BSON objects).
	 */
	if (node->type == BSON_EOO) {
		PX4_DEBUG("end of parameters");
		return 0;
	}

	param_modify_on_import(node);

	// Find the parameter this node represents.  If we don't know it, ignore the node.
	param_t param = param_find_no_notification(node->name);

	if (param == PARAM_INVALID) {
		PX4_WARN("ignoring unrecognised parameter '%s'", node->name);
		return 1;
	}

	// Handle setting the parameter from the node
	switch (node->type) {
	case BSON_INT32: {
			if (param_type(param) == PARAM_TYPE_INT32) {
				int32_t i = node->i32;
				param_set_internal(param, &i, true, true);
				PX4_DEBUG("Imported %s with value %" PRIi32, param_name(param), i);

			} else {
				PX4_WARN("unexpected type for %s", node->name);
			}
		}
		break;

	case BSON_DOUBLE: {
			if (param_type(param) == PARAM_TYPE_FLOAT) {
				float f = node->d;
				param_set_internal(param, &f, true, true);
				PX4_DEBUG("Imported %s with value %f", param_name(param), (double)f);

			} else {
				PX4_WARN("unexpected type for %s", node->name);
			}
		}
		break;

	default:
		PX4_ERR("import: unrecognised node type for '%s'", node->name);
	}

	// don't return zero, that means EOF
	return 1;
}

static int
param_import_internal(int fd)
{
	static constexpr int MAX_ATTEMPTS = 3;

	for (int attempt = 1; attempt <= MAX_ATTEMPTS; attempt++) {
		bson_decoder_s decoder{};

		if (bson_decoder_init_file(&decoder, fd, param_import_callback) == 0) {
			int result = -1;

			do {
				result = bson_decoder_next(&decoder);

			} while (result > 0);

			if (result == 0) {
				if (decoder.total_document_size == decoder.total_decoded_size) {
					PX4_INFO("BSON document size %" PRId32 " bytes, decoded %" PRId32 " bytes (INT32:%" PRIu16 ", FLOAT:%" PRIu16 ")",
						 decoder.total_document_size, decoder.total_decoded_size,
						 decoder.count_node_int32, decoder.count_node_double);

					return 0;

				} else {
					PX4_ERR("BSON document size (%" PRId32 ") doesn't match bytes decoded (%" PRId32 ")",
						decoder.total_document_size, decoder.total_decoded_size);
				}

			} else if (result == -ENODATA) {
				// silently retry as a precaution unless this is our last attempt
				if (attempt == MAX_ATTEMPTS) {
					PX4_DEBUG("BSON: no data");
					return 0;
				}

			} else {
				PX4_ERR("param import failed (%d) attempt %d", result, attempt);
			}

		} else {
			PX4_ERR("param import bson decoder init failed (attempt %d)", attempt);
		}

		if (attempt != MAX_ATTEMPTS) {
			if (lseek(fd, 0, SEEK_SET) != 0) {
				PX4_ERR("import lseek failed (%d)", errno);
			}

			usleep(10000); // wait at least 10 milliseconds before trying again
		}
	}

	return -1;
}

int
param_import(int fd)
{
	if (fd < 0) {
		return flash_param_import();
	}

	return param_import_internal(fd);
}

int
param_load(int fd)
{
	if (fd < 0) {
		return flash_param_load();
	}

	param_reset_all_internal(false);
	return param_import_internal(fd);
}

void
param_foreach(void (*func)(void *arg, param_t param), void *arg, bool only_changed, bool only_used)
{
	param_t	param;

	for (param = 0; handle_in_range(param); param++) {

		/* if requested, skip unchanged values */
		if (only_changed && (param_find_changed(param) == nullptr)) {
			continue;
		}

		if (only_used && !param_used(param)) {
			continue;
		}

		func(arg, param);
	}
}

uint32_t param_hash_check()
{
	uint32_t param_hash = 0;

	param_lock_reader();

	/* compute the CRC32 over all string param names and 4 byte values */
	for (param_t param = 0; handle_in_range(param); param++) {
		if (!param_used(param) || param_is_volatile(param)) {
			continue;
		}

		const char *name = param_name(param);
		const void *val = param_get_value_ptr(param);
		param_hash = crc32part((const uint8_t *)name, strlen(name), param_hash);
		param_hash = crc32part((const uint8_t *)val, param_size(param), param_hash);
	}

	param_unlock_reader();

	return param_hash;
}

void param_print_status()
{
	PX4_INFO("summary: %d/%d (used/total)", param_count_used(), param_count());

#ifndef FLASH_BASED_PARAMS
	const char *filename = param_get_default_file();

	if (filename != nullptr) {
		PX4_INFO("file: %s", param_get_default_file());
	}

	if (param_backup_file) {
		PX4_INFO("backup file: %s", param_backup_file);
	}

#endif /* FLASH_BASED_PARAMS */

	if (param_values != nullptr) {
		PX4_INFO("storage array: %d/%d elements (%zu bytes total)",
			 utarray_len(param_values), param_values->n, param_values->n * sizeof(UT_icd));
	}

	if (param_custom_default_values != nullptr) {
		PX4_INFO("storage array (custom defaults): %d/%d elements (%zu bytes total)",
			 utarray_len(param_custom_default_values), param_custom_default_values->n,
			 param_custom_default_values->n * sizeof(UT_icd));
	}

	PX4_INFO("auto save: %s", autosave_disabled ? "off" : "on");

	if (!autosave_disabled && (last_autosave_timestamp > 0)) {
		PX4_INFO("last auto save: %.3f seconds ago", hrt_elapsed_time(&last_autosave_timestamp) * 1e-6);
	}

	perf_print_counter(param_export_perf);
	perf_print_counter(param_find_perf);
	perf_print_counter(param_get_perf);
	perf_print_counter(param_set_perf);
}
