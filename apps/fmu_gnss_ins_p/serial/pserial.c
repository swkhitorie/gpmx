#include "pserial.h"

#include <string.h>
#include <errno.h>

#include <termios.h>
#include <linux/serial.h>

#ifndef CRTSCTS	/* flow control */
#define CRTSCTS	  020000000000	/* flow control */
#endif

struct termios config;

int serial_setspeed(struct termios *cfg, int speed)
{
    unsigned int i = 0;
    int speed_attr[] = {
        B921600, B576000, B500000, B460800, B230400, B115200, B57600, 
        B38400, B19200, B9600, B4800, B2400, B1200, B300, 
    };
    int name_attr[] = {
        921600, 576000, 500000, 460800, 230400, 115200, 57600,
        38400, 19200, 9600, 4800, 2400, 1200, 300
    };

    for (; i < sizeof(speed_attr) / sizeof(int); i++) {
        if(speed == name_attr[i]) {
            cfsetispeed(cfg, speed_attr[i]);
            cfsetospeed(cfg, speed_attr[i]);
            return 1;
        }
    }
    if (i == sizeof(speed_attr) / sizeof(int)) {
        cfsetispeed(cfg, B115200);
        cfsetospeed(cfg, B115200);
    }
    return 0;
}

int serial_setparity(struct termios *cfg, int databits, int stopbits, int parity)
{
    // Turn on READ & ignore ctrl lines (CLOCAL = 1)
    cfg->c_cflag |= (CLOCAL | CREAD);

    // Disable RTS/CTS hardware flow control (most common)
    cfg->c_cflag &= ~CRTSCTS;

    switch (databits) {
    case 7:
        cfg->c_cflag &= ~CSIZE;
        cfg->c_cflag |= CS7;
        break;
    case 8:
        cfg->c_cflag &= ~CSIZE;
        cfg->c_cflag |= CS8;
        break;
    default:
        printf("[%s] unsupported datasize \n", __func__);
        return -1;
    }

    switch (parity) {
    case 'n':
    case 'N':
        cfg->c_cflag &= ~PARENB; //clear parity
        cfg->c_iflag &= ~INPCK;  //enable parity checking
        break;
    case 'o':
    case 'O':
        cfg->c_cflag |= PARENB;  //enable parity
        cfg->c_cflag |= PARODD;
        cfg->c_iflag |= INPCK;   //disable parity checking
        break;
    case 'e':
    case 'E':
        cfg->c_cflag |= PARENB;   //enable parity         
        cfg->c_cflag &= ~PARODD;        
        cfg->c_iflag |= INPCK;   //disable pairty checking        
        break;    
    default:
        printf("[%s] unsupported parity \n", __func__);
        return -1;       
    }    

    switch (stopbits) {
    case 1:
        cfg->c_cflag &= ~CSTOPB;
        break;     
    case 2:
        cfg->c_cflag |= CSTOPB;
        break;     
    default:
        printf("[%s] unsupported stopbits \n", __func__);        
        return -1;
    }

    cfg->c_cflag |= (CLOCAL | CREAD);
    // Disable echo, erasure, new-line echo, interpretation of INTR, QUIT and SUSP
    cfg->c_lflag &= ~(ICANON | IEXTEN | ECHO | ECHOE | ECHONL | ISIG);

    // Prevent special interpretation of output bytes (e.g. newline chars)
    cfg->c_oflag &= ~OPOST;
    // Prevent conversion of newline to carriage return/line feed
    cfg->c_oflag &= ~(ONLCR | OCRNL);
	// tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
	// tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    // Turn off s/w flow ctrl
    cfg->c_iflag &= ~(IXON | IXOFF | IXANY);
    // Disable any special handling of received bytes
    cfg->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | INPCK);
    // cfg->c_iflag &= ~(ICRNL | INLCR);
    return 0;
}

int pserial_setup(const char *dev, int baud)
{
    int fd = open(dev, O_RDWR|O_NOCTTY); //O_NONBLOCK O_NDELAY

	if (tcgetattr(fd, &config) != 0) {
        printf("[%s] error %d:%s \n", __func__ ,errno, strerror(errno));
        close(fd);
        return -1;
    }

    bzero(&config, sizeof(struct termios));
    serial_setspeed(&config, baud);
    serial_setparity(&config, 8, 1, 'N');

    // Wait for up to 1/10s (10 deciseconds), returning as soon as any data is received.
	config.c_cc[VTIME] = 0;    
	config.c_cc[VMIN] = 0;

    // enable raw mode
    cfmakeraw(&config);

    tcflush(fd, TCIOFLUSH);

	if (tcsetattr(fd, TCSANOW, &config) != 0) {
        printf("[%s] error %d:%s \n", __func__ ,errno, strerror(errno));
        close(fd);
        return -1;
    }
    return fd;
}

int pserial_baudset(int dev, int baud)
{
    tcgetattr(dev, &config);
    serial_setspeed(&config, baud);
    if (tcsetattr(dev, TCSADRAIN, &config) != 0) {
        printf("[%s] error %d:%s \n", __func__ ,errno, strerror(errno));
        close(dev);
        return -1;
    }
    tcflush(dev, TCIOFLUSH);
    return 0;
}

#include <termios.h>
#include <sys/epoll.h>
#include <sched.h>
static int serial_dev = 0;

void *thread_config_rcv(void* arg)
{
	int i, j, ret;

    const uint32_t rcv_capacity = 4*1024;
	unsigned char rcv_buf[rcv_capacity];
	memset(&rcv_buf[0], 0, sizeof(rcv_buf));

	int rd_sz = 0;
	int epfd, nfds;
    struct epoll_event event;
    struct epoll_event* events;

    events = calloc(10, sizeof(event));  
    epfd = epoll_create(10);

    event.data.fd = serial_dev;
    event.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serial_dev, &event);

	for(;;) 
    {
		ret = epoll_wait(epfd, events, 10, -1);// -1 :wait until it happen
		for (i = 0; i < ret; i++) {
			if (events[i].data.fd == serial_dev) {
                // printf("active\n");
				rd_sz = read(events[i].data.fd, rcv_buf, rcv_capacity);
				if (rd_sz < 0){
                    printf("error reading: %s \r\n", strerror(errno));
				}

                for (int j = 0; j < rd_sz; j++) {
                    printf("%c", rcv_buf[j]);
                    // t5_config_rcv_process(rcv_buf[j]);
                }
				rd_sz = 0;
			}
		}
    }
	close(serial_dev);
}

void pserial_recving_start(const char *path)
{
    int i, ret;
    pthread_t cfg_rcv_id;
    serial_dev = pserial_setup(path, 921600);

    if (serial_dev < 0) {
        return;
    }

	ret = pthread_create(&cfg_rcv_id, NULL, thread_config_rcv, NULL);
	if (ret != 0) {
        printf("[pserial] create recv thread failed \n");
		return;
	}

    printf("[pserial] recv start\n");
    pthread_detach(cfg_rcv_id);
}
