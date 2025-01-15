#ifndef POSIX_IOCTL_H_
#define POSIX_IOCTL_H_

#define _IOC_MASK       (0x00ff)
#define _IOC_TYPE(cmd)  ((cmd) & ~_IOC_MASK)
#define _IOC_NR(cmd)    ((cmd) & _IOC_MASK)

#define _IOC(type,nr)   ((type)|(nr))

#endif
