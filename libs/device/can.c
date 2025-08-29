#include <device/can.h>

#define CAN_DEVNAME_FMT    "/dev/can%d"
#define CAN_DEVNAME_FMTLEN (8 + 3 + 1)

int can_register(struct can_dev_s *dev, int bus)
{
    int ret = 0;

    char devname[CAN_DEVNAME_FMTLEN];
    snprintf(devname, CAN_DEVNAME_FMTLEN, CAN_DEVNAME_FMT, bus);

    ret = dn_register(devname, dev);

    return ret;
}

struct can_dev_s* can_bus_get(int bus)
{
    struct can_dev_s *dev;
    char devname[CAN_DEVNAME_FMTLEN];
    snprintf(devname, CAN_DEVNAME_FMTLEN, CAN_DEVNAME_FMT, bus);

    dev = dn_bind(devname);

    return dev;
}

int can_bus_initialize(int bus)
{
    struct can_dev_s *dev = can_bus_get(bus);

    if (!dev) {
        return -1;
    }

#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    dev->sem_tx = xSemaphoreCreateBinary();
    xSemaphoreGive(dev->sem_tx);
#else

    dev->flag_tx = 0x01;
#endif

    return dev->cd_ops->co_setup(dev);
}

int can_tx_wait(struct can_dev_s *dev)
{
    int ret = DTRUE;
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    xSemaphoreTake(dev->sem_tx, portMAX_DELAY);
#else

    while (dev->flag_tx != 0x01);
    dev->flag_tx = 0x00;
#endif
    return ret;
}

void can_tx_post(struct can_dev_s *dev)
{
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)

    BaseType_t h_pri;
    xSemaphoreGiveFromISR(dev->sem_tx, &h_pri);
    portYIELD_FROM_ISR(h_pri);
#else

    dev->flag_tx = 0x01;
#endif
}

void can_rxfifo_clear(struct can_rxfifo_s *rfifo)
{
    dn_disable_irq();

    rfifo->rx_head = 0;
    rfifo->rx_tail = 0;

    dn_enable_irq();
}

int can_rxfifo_put(struct can_rxfifo_s *rfifo, void *buf)
{
    int ret = DTRUE;

    dn_disable_irq();

    struct can_msg_s *pmsg;
    pmsg = (struct can_msg_s *)buf;

    if(((rfifo->rx_tail + 1) % CONFIG_CAN_FIFOSIZE == rfifo->rx_head)) {
        // full
        ret = DFALSE; 
    } else {
        SMEMCPY(&rfifo->rx_buffer[rfifo->rx_tail], pmsg, sizeof(struct can_msg_s));

        rfifo->rx_tail = (rfifo->rx_tail + 1) % CONFIG_CAN_FIFOSIZE;
    }

    dn_enable_irq();

    return ret;
}

int can_rxfifo_get(struct can_rxfifo_s *rfifo, void *buf)
{
    int ret = DTRUE;

    dn_disable_irq();

    if(rfifo->rx_tail == rfifo->rx_head) {
        // empty
        ret = DFALSE;
    } else {
        SMEMCPY(buf, &rfifo->rx_buffer[rfifo->rx_head], sizeof(struct can_msg_s));

        rfifo->rx_head = (rfifo->rx_head + 1) % CONFIG_CAN_FIFOSIZE;
    }

    dn_enable_irq();

    return ret;
}
