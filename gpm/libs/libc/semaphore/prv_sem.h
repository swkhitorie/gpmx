#ifndef PRIVATE_SEM_H_
#define PRIVATE_SEM_H_

#include <gpmx/config.h>
#include <semaphore.h>

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

int posix_sem_system_init();
void posix_sem_insert(sem_t *psem);
void posix_sem_delete(sem_t *psem);
sem_t *posix_sem_find(const char* name);
void posix_sem_lock();
void posix_sem_unlock();

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif
