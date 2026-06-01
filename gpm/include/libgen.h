
#ifndef __INCLUDE_LIBGEN_H
#define __INCLUDE_LIBGEN_H

#include <sys/types.h>

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

char *basename(char *path);
char *dirname(char *path);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif
