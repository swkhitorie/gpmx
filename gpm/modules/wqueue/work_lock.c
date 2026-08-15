
#include <gpmx/config.h>
#include <stdio.h>
#include <semaphore.h>
#include "work_lock.h"

extern sem_t _work_lock[];

void work_lock(int id)
{
	sem_wait(&_work_lock[id]);
}

void work_unlock(int id)
{
	sem_post(&_work_lock[id]);
}
