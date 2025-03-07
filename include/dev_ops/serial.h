#ifndef DEV_OPS_SERIAL_H_
#define DEV_OPS_SERIAL_H_


struct uart_buffer_s
{
    volatile int16_t head;   /* Index to the head [IN] index in the buffer */
    volatile int16_t tail;   /* Index to the tail [OUT] index in the buffer */
    int16_t          size;   /* The allocated size of the buffer */
    char        *buffer; /* Pointer to the allocated buffer memory */
};

struct uart_dmaxfer_s {
    char    *buffer;
    size_t   capacity;
    char     halflag;
};

/* This structure defines all of the operations providd by the architecture specific
* logic.  All fields must be provided with non-NULL function pointers by the
* caller of uart_register().
*/

struct uart_dev_s;
struct uart_ops_s
{
/* Configure the UART baud, bits, parity, fifos, etc. This method is called
* the first time that the serial port is opened.  For the serial console,
* this will occur very early in initialization; for other serial ports this
* will occur when the port is first opened.  This setup does not include
* attaching or enabling interrupts.  That portion of the UART setup is
* performed when the attach() method is called.
*/

CODE int (*setup)(FAR struct uart_dev_s *dev);

/* Disable the UART.  This method is called when the serial port is closed.
* This method reverses the operation the setup method.  NOTE that the serial
* console is never shutdown.
*/

CODE void (*shutdown)(FAR struct uart_dev_s *dev);

/* Configure the UART to operation in interrupt driven mode.  This method is
* called when the serial port is opened.  Normally, this is just after the
* the setup() method is called, however, the serial console may operate in
* a non-interrupt driven mode during the boot phase.
*
* RX and TX interrupts are not enabled when by the attach method (unless the
* hardware supports multiple levels of interrupt enabling).  The RX and TX
* interrupts are not enabled until the txint() and rxint() methods are called.
*/

CODE int (*attach)(FAR struct uart_dev_s *dev);

/* Detach UART interrupts.  This method is called when the serial port is
* closed normally just before the shutdown method is called.  The exception is
* the serial console which is never shutdown.
*/

CODE void (*detach)(FAR struct uart_dev_s *dev);

/* All ioctl calls will be routed through this method */

CODE int (*ioctl)(FAR struct file *filep, int cmd, unsigned long arg);

/* Called (usually) from the interrupt level to receive one character from
* the UART.  Error bits associated with the receipt are provided in the
* the return 'status'.
*/

CODE int (*receive)(FAR struct uart_dev_s *dev, FAR unsigned int *status);

/* Call to enable or disable RX interrupts */

CODE void (*rxint)(FAR struct uart_dev_s *dev, bool enable);

/* Return true if the receive data is available */

CODE bool (*rxavailable)(FAR struct uart_dev_s *dev);

#ifdef CONFIG_SERIAL_IFLOWCONTROL
/* Return true if UART activated RX flow control to block more incoming
* data.
*/

CODE bool (*rxflowcontrol)(FAR struct uart_dev_s *dev,
                            unsigned int nbuffered, bool upper);
#endif

#ifdef CONFIG_SERIAL_TXDMA
/* Start transfer bytes from the TX circular buffer using DMA */

CODE void (*dmasend)(FAR struct uart_dev_s *dev);
#endif

#ifdef CONFIG_SERIAL_RXDMA
/* Start transfer bytes from the TX circular buffer using DMA */

CODE void (*dmareceive)(FAR struct uart_dev_s *dev);

/* Notify DMA that there is free space in the RX buffer */

CODE void (*dmarxfree)(FAR struct uart_dev_s *dev);
#endif

#ifdef CONFIG_SERIAL_TXDMA
/* Notify DMA that there is data to be transferred in the TX buffer */

CODE void (*dmatxavail)(FAR struct uart_dev_s *dev);
#endif

/* This method will send one byte on the UART */

CODE void (*send)(FAR struct uart_dev_s *dev, int ch);

/* Call to enable or disable TX interrupts */

CODE void (*txint)(FAR struct uart_dev_s *dev, bool enable);

/* Return true if the tranmsit hardware is ready to send another byte.  This
* is used to determine if send() method can be called.
*/

CODE bool (*txready)(FAR struct uart_dev_s *dev);

/* Return true if all characters have been sent.  If for example, the UART
* hardware implements FIFOs, then this would mean the transmit FIFO is
* empty.  This method is called when the driver needs to make sure that
* all characters are "drained" from the TX hardware.
*/

CODE bool (*txempty)(FAR struct uart_dev_s *dev);
};

priv{

    unsigned char devid;
    unsigned char pin_tx;
    unsigned char pin_rx;
    unsigned short priority;
    unsigned short priority_dmatx;
    unsigned short priority_dmarx;
}
struct uart_dev_s {

    /* State data */
    unsigned char tx_ready;

    /* Setting data */
    int baudrate;
    unsigned char wordlen;
    unsigned char stopbitlen;
    char parity;

    /* I/O buffers */
    struct uart_buffer_s xmit;    /* Describes transmit buffer */
    struct uart_buffer_s recv;    /* Describes receive buffer */

    /* DMA transfers */
    struct uart_dmaxfer_s dmatx;  /* Describes transmit DMA transfer */
    struct uart_dmaxfer_s dmarx;  /* Describes receive DMA transfer */

    /* Driver interface */
    const struct uart_ops_s *ops;  /* Arch-specific operations */
    void                    *priv; /* Used by the arch-specific logic */
};

typedef struct uart_dev_s uart_dev_t;

#endif
