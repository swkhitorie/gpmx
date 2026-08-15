
#ifndef __INCLUDE_BUILTIN_APP_H
#define __INCLUDE_BUILTIN_APP_H

#include <stdint.h>

#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/************************************
 * BUILD APP Stask
 ************************************/

int     built_task_spawn(const char *name, int scheduler, int priority, 
    int stack_size, int (*entry)(int argc, char **argv), char *const argv[], bool joinable);
int     built_task_delete(int id);
void    built_task_exit(int ret);
int     built_task_kill(int id, int sig);
int     built_task_join(int id);
void    built_show_tasks();
bool    built_task_is_running(const char *taskname);
int     built_getpid();
const char *built_get_taskname();

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif

