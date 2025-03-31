#ifndef DEV_OPS_CAN_H_
#define DEV_OPS_CAN_H_

#include <stdint.h>
#include <stdbool.h>

#define CAN_MAXDATALEN         64  /* canfd-64, canstandrd-8 */

#define CONFIG_CAN_FIFOSIZE    32

struct can_hdr_s
{
    uint32_t     ch_id;         /* 11- or 29-bit ID (20- or 3-bits unused) */
    uint8_t      ch_dlc    : 4; /* 4-bit DLC (data filed length) */
    uint8_t      ch_rtr    : 1; /* RTR indication, rtr=0:dataframe, rtr=1:remoteframe */
    uint8_t      ch_error  : 1; /* 1=ch_id is an error report */
    uint8_t      ch_extid  : 1; /* Extended ID indication, 0:normal id, 1:extend id*/
    uint8_t      ch_unused : 1; /* Unused */
};

struct can_msg_s
{
    struct can_hdr_s cm_hdr;                  /* The CAN header */
    uint8_t          cm_data[CAN_MAXDATALEN]; /* CAN message data (0-8 byte) */
};

struct can_rxfifo_s
{
    // sem_t         rx_sem;                  /* Counting semaphore */
    uint8_t       rx_head;                 /* Index to the head [IN] in the circular buffer */
    uint8_t       rx_tail;                 /* Index to the tail [OUT] in the circular buffer */
                                            /* Circular buffer of CAN messages */
    struct can_msg_s rx_buffer[CONFIG_CAN_FIFOSIZE];
};

struct can_txfifo_s
{
    // sem_t         tx_sem;                  /* Counting semaphore */
    uint8_t       tx_head;                 /* Index to the head [IN] in the circular buffer */
    uint8_t       tx_queue;                /* Index to next message to send */
    uint8_t       tx_tail;                 /* Index to the tail [OUT] in the circular buffer */
                                            /* Circular buffer of CAN messages */
    struct can_msg_s tx_buffer[CONFIG_CAN_FIFOSIZE];
};

struct can_rtrwait_s
{
    // sem_t         cr_sem;                  /* Wait for RTR response */
    uint16_t      cr_id;                   /* The ID that is waited for */
    struct can_msg_s *cr_msg;          /* This is where the RTR response goes */
};

struct canioc_bittiming_s
{
    uint32_t              bt_baud;         /* Bit rate = 1 / bit time */
    uint8_t               bt_tseg1;        /* TSEG1 in time quanta */
    uint8_t               bt_tseg2;        /* TSEG2 in time quanta */
    uint8_t               bt_sjw;          /* Synchronization Jump Width in time quanta */
};

struct can_dev_s;
struct can_ops_s
{
    /* Reset the CAN device. */
    void (*co_reset)(struct can_dev_s *dev);

    /* Configure the CAN.*/
    int (*co_setup)(struct can_dev_s *dev);

    /* Disable the CAN. */
    void (*co_shutdown)(struct can_dev_s *dev);

    /* All ioctl calls will be routed through this method */

    int (*co_ioctl)(struct can_dev_s *dev, int cmd, unsigned long arg);

    /* Send a remote request */
    int (*co_remoterequest)(struct can_dev_s *dev, uint16_t id);

    /* This method will send one message on the CAN */
    int (*co_send)(struct can_dev_s *dev, struct can_msg_s *msg);

    /* Return true if the CAN hardware can accept another TX message. */
    bool (*co_txready)(struct can_dev_s *dev);

    /* Return true if all message have been sent.  If for example, the CAN
    * hardware implements FIFOs, then this would mean the transmit FIFO is
    * empty.  This method is called when the driver needs to make sure that
    * all characters are "drained" from the TX hardware before calling co_shutdown().
    */
    bool (*co_txempty)(struct can_dev_s *dev);
};

struct can_dev_s
{
    uint32_t bus_baud;
    struct canioc_bittiming_s bittiming;

    uint8_t    cd_error;  /* Flags to indicate internal device errors */

    struct can_txfifo_s  cd_xmit;
    struct can_rxfifo_s  cd_rcv;

    const struct can_ops_s *cd_ops;    /* Arch-specific operations */
    void            *cd_priv;          /* Used by the arch-specific logic */
};

#if defined(__cplusplus)
extern "C"{
#endif



#if defined(__cplusplus)
}
#endif

#endif
