#ifndef PSERIAL_H_
#define PSERIAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/select.h>


#ifdef __cplusplus
extern "C" {
#endif

int  pserial_setup(const char *dev, int baud);
int  pserial_baudset(int dev, int baud);
void pserial_recving_start(const char *path);

#ifdef __cplusplus
}
#endif

#endif
