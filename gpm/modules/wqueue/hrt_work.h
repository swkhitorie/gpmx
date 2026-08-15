
#include <gpmx/config.h>
#include <semaphore.h>
#include <wqueue.h>

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern sem_t _hrt_work_lock;
extern struct wqueue_s g_hrt_work;

void hrt_work_queue_init(void);
int hrt_work_queue(struct work_s *work, worker_t worker, void *arg, uint32_t usdelay);
void hrt_work_cancel(struct work_s *work);

static inline void hrt_work_lock(void);
static inline void hrt_work_lock()
{
	sem_wait(&_hrt_work_lock);
}

static inline void hrt_work_unlock(void);
static inline void hrt_work_unlock()
{
	sem_post(&_hrt_work_lock);
}

#ifdef __cplusplus
}
#endif
