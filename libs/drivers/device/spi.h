#ifndef DEV_OPS_SPI_H_
#define DEV_OPS_SPI_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#if defined(CONFIG_BOARD_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <semphr.h>
#endif

#include "dnode.h"

/****************************************************************************
 * Name: SPI_LOCK
 *
 * Description:
 *   On SPI buses where there are multiple devices, it will be necessary to
 *   lock SPI to have exclusive access to the buses for a sequence of
 *   transfers.  The bus should be locked before the chip is selected. After
 *   locking the SPI bus, the caller should then also call the setfrequency,
 *   setbits, and setmode methods to make sure that the SPI is properly
 *   configured for the device.  If the SPI buss is being shared, then it
 *   may have been left in an incompatible state.
 *
 * Input Parameters:
 *   dev  - Device-specific state data
 *   lock - true: Lock spi bus, false: unlock SPI bus
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

#define SPI_LOCK(d,l) (d)->ops->lock(d,l)

/****************************************************************************
 * Name: SPI_SELECT
 *
 * Description:
 *   Enable/disable the SPI chip select.   The implementation of this method
 *   must include handshaking:  If a device is selected, it must hold off
 *   all other attempts to select the device until the device is deselected.
 *   Required.
 *
 * Input Parameters:
 *   dev -      Device-specific state data
 *   devid -    Identifies the device to select
 *   selected - true: slave selected, false: slave de-selected
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

#define SPI_SELECT(d,id,s) ((d)->ops->select(d,id,s))

/****************************************************************************
 * Name: SPI_SETFREQUENCY
 *
 * Description:
 *   Set the SPI frequency. Required.
 *
 * Input Parameters:
 *   dev -       Device-specific state data
 *   frequency - The SPI frequency requested
 *
 * Returned Value:
 *   Returns the actual frequency selected
 *
 ****************************************************************************/

#define SPI_SETFREQUENCY(d,f) ((d)->ops->setfrequency(d,f))

/****************************************************************************
 * Name: SPI_SETMODE
 *
 * Description:
 *   Set the SPI mode. Optional.  See enum spi_mode_e for mode definitions.
 *
 * Input Parameters:
 *   dev -  Device-specific state data
 *   mode - The SPI mode requested
 *
 * Returned Value:
 *   none
 *
 ****************************************************************************/

#define SPI_SETMODE(d,m) \
    do { if ((d)->ops->setmode) (d)->ops->setmode(d,m); } while (0)

/****************************************************************************
 * Name: SPI_SETBITS
 *
 * Description:
 *   Set the number if bits per word.
 *
 * Input Parameters:
 *   dev -  Device-specific state data
 *   nbits - The number of bits in an SPI word.
 *
 * Returned Value:
 *   none
 *
 ****************************************************************************/

#define SPI_SETBITS(d,b) \
    do { if ((d)->ops->setbits) (d)->ops->setbits(d,b); } while (0)

/****************************************************************************
 * Name: SPI_SNDBLOCK
 *
 * Description:
 *   Send a block of data on SPI. Required.
 *
 * Input Parameters:
 *   dev    - Device-specific state data
 *   buffer - A pointer to the buffer of data to be sent
 *   nwords - the length of data to send from the buffer in number of words.
 *            The wordsize is determined by the number of bits-per-word
 *            selected for the SPI interface.  If nbits <= 8, the data is
 *            packed into uint8_t's; if nbits >8, the data is packed into
 *            uint16_t's
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

#define SPI_SNDBLOCK(d,b,l) ((d)->ops->sndblock(d,b,l))

/****************************************************************************
 * Name: SPI_RECVBLOCK
 *
 * Description:
 *   Receive a block of data from SPI. Required.
 *
 * Input Parameters:
 *   dev -    Device-specific state data
 *   buffer - A pointer to the buffer in which to receive data
 *   nwords - the length of data that can be received in the buffer in number
 *            of words.  The wordsize is determined by the number of bits-
 *            per-word selected for the SPI interface.  If nbits <= 8, the
 *            data is packed into uint8_t's; if nbits >8, the data is packed
 *            into uint16_t's
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

#define SPI_RECVBLOCK(d,b,l) ((d)->ops->recvblock(d,b,l))

/****************************************************************************
 * Name: SPI_EXCHANGE
 *
 * Description:
 *   Exchange a block of data from SPI. Required.
 *
 * Input Parameters:
 *   dev      - Device-specific state data
 *   txbuffer - A pointer to the buffer of data to be sent
 *   rxbuffer - A pointer to the buffer in which to receive data
 *   nwords   - the length of data that to be exchanged in units of words.
 *              The wordsize is determined by the number of bits-per-word
 *              selected for the SPI interface.  If nbits <= 8, the data is
 *              packed into uint8_t's; if nbits >8, the data is packed into
 *              uint16_t's
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

#define SPI_EXCHANGE(d,t,r,l) ((d)->ops->exchange(d,t,r,l))

#define SPI_EXCHANGEBLOCK(d,t,r,l) ((d)->ops->exchangeblock(d,t,r,l))

/* SPI Device Macros ********************************************************/

/* This builds a SPI devid from its type and index */

#define SPIDEV_ID(type,index) ((((uint32_t)(type)  & 0xffff) << 16) | \
                                ((uint32_t)(index) & 0xffff))

/* This retrieves the fields from a SPI devid */

#define SPIDEVID_TYPE (devid) (((uint32_t)(devid) >> 16) & 0xffff)
#define SPIDEVID_INDEX(devid)  ((uint32_t)(devid)        & 0xffff)

/* These are standard definitions for the defined SPI device IDs.  The index
 * argument, n, is the instance number.  This should be zero if there is
 * only one instance of the SPI device on the SPI bus. Indices greater than
 * zero discriminate the additional devices of the same type on the SPI bus.
 */

#define SPIDEV_NONE(n)          SPIDEV_ID(SPIDEVTYPE_NONE,          (n))
#define SPIDEV_MMCSD(n)         SPIDEV_ID(SPIDEVTYPE_MMCSD,         (n))
#define SPIDEV_FLASH(n)         SPIDEV_ID(SPIDEVTYPE_FLASH,         (n))
#define SPIDEV_ETHERNET(n)      SPIDEV_ID(SPIDEVTYPE_ETHERNET,      (n))
#define SPIDEV_DISPLAY(n)       SPIDEV_ID(SPIDEVTYPE_DISPLAY,       (n))
#define SPIDEV_CAMERA(n)        SPIDEV_ID(SPIDEVTYPE_CAMERA,        (n))
#define SPIDEV_WIRELESS(n)      SPIDEV_ID(SPIDEVTYPE_WIRELESS,      (n))
#define SPIDEV_TOUCHSCREEN(n)   SPIDEV_ID(SPIDEVTYPE_TOUCHSCREEN,   (n))
#define SPIDEV_EXPANDER(n)      SPIDEV_ID(SPIDEVTYPE_EXPANDER,      (n))
#define SPIDEV_MUX(n)           SPIDEV_ID(SPIDEVTYPE_MUX,           (n))
#define SPIDEV_AUDIO_DATA(n)    SPIDEV_ID(SPIDEVTYPE_AUDIO_DATA,    (n))
#define SPIDEV_AUDIO_CTRL(n)    SPIDEV_ID(SPIDEVTYPE_AUDIO_CTRL,    (n))
#define SPIDEV_EEPROM(n)        SPIDEV_ID(SPIDEVTYPE_EEPROM,        (n))
#define SPIDEV_ACCELEROMETER(n) SPIDEV_ID(SPIDEVTYPE_ACCELEROMETER, (n))
#define SPIDEV_BAROMETER(n)     SPIDEV_ID(SPIDEVTYPE_BAROMETER,     (n))
#define SPIDEV_TEMPERATURE(n)   SPIDEV_ID(SPIDEVTYPE_TEMPERATURE,   (n))
#define SPIDEV_IEEE802154(n)    SPIDEV_ID(SPIDEVTYPE_IEEE802154,    (n))
#define SPIDEV_CONTACTLESS(n)   SPIDEV_ID(SPIDEVTYPE_CONTACTLESS,   (n))
#define SPIDEV_CANBUS(n)        SPIDEV_ID(SPIDEVTYPE_CANBUS,        (n))
#define SPIDEV_USBHOST(n)       SPIDEV_ID(SPIDEVTYPE_USBHOST,       (n))
#define SPIDEV_LPWAN(n)         SPIDEV_ID(SPIDEVTYPE_LPWAN,         (n))
#define SPIDEV_ADC(n)           SPIDEV_ID(SPIDEVTYPE_ADC,           (n))
#define SPIDEV_USER(n)          SPIDEV_ID(SPIDEVTYPE_USER,          (n))

/* If the board supports multiple SPI devices types, this enumeration
 * identifies which is selected or de-selected.
 * There may be more than one instance of each type on a bus, see below.
 */

enum spi_devtype_e
{
  SPIDEVTYPE_NONE = 0,      /* Not a valid value */
  SPIDEVTYPE_MMCSD,         /* Select SPI MMC/SD device */
  SPIDEVTYPE_FLASH,         /* Select SPI FLASH device */
  SPIDEVTYPE_ETHERNET,      /* Select SPI Ethernet device */
  SPIDEVTYPE_DISPLAY,       /* Select SPI LCD/OLED display device */
  SPIDEVTYPE_CAMERA,        /* Select SPI imaging device */
  SPIDEVTYPE_WIRELESS,      /* Select SPI Wireless device */
  SPIDEVTYPE_TOUCHSCREEN,   /* Select SPI touchscreen device */
  SPIDEVTYPE_EXPANDER,      /* Select SPI I/O expander device */
  SPIDEVTYPE_MUX,           /* Select SPI multiplexer device */
  SPIDEVTYPE_AUDIO_DATA,    /* Select SPI audio codec device data port */
  SPIDEVTYPE_AUDIO_CTRL,    /* Select SPI audio codec device control port */
  SPIDEVTYPE_EEPROM,        /* Select SPI EEPROM device */
  SPIDEVTYPE_ACCELEROMETER, /* Select SPI Accelerometer device */
  SPIDEVTYPE_BAROMETER,     /* Select SPI Pressure/Barometer device */
  SPIDEVTYPE_TEMPERATURE,   /* Select SPI Temperature sensor device */
  SPIDEVTYPE_IEEE802154,    /* Select SPI IEEE 802.15.4 wireless device */
  SPIDEVTYPE_CONTACTLESS,   /* Select SPI Contactless device */
  SPIDEVTYPE_CANBUS,        /* Select SPI CAN bus controller over SPI */
  SPIDEVTYPE_USBHOST,       /* Select SPI USB host controller over SPI */
  SPIDEVTYPE_LPWAN,         /* Select SPI LPWAN controller over SPI */
  SPIDEVTYPE_ADC,           /* Select SPI ADC device */
  SPIDEVTYPE_USER           /* Board-specific values start here
                             * This must always be the last definition. */
};

/* Certain SPI devices may required different clocking modes */

enum spi_mode_e
{
  SPIDEV_MODE0 = 0,     /* CPOL=0 CHPHA=0 */
  SPIDEV_MODE1,         /* CPOL=0 CHPHA=1 */
  SPIDEV_MODE2,         /* CPOL=1 CHPHA=0 */
  SPIDEV_MODE3,         /* CPOL=1 CHPHA=1 */
  SPIDEV_MODETI,        /* CPOL=0 CPHA=1 TI Synchronous Serial Frame Format */
};

struct spi_dev_s;
struct spi_ops_s
{
    int (*setup)(struct spi_dev_s *dev);

    uint32_t (*setfrequency)(struct spi_dev_s *dev, uint32_t frequency);

    void (*setmode)(struct spi_dev_s *dev, enum spi_mode_e mode);

    void (*setbits)(struct spi_dev_s *dev, int nbits);

    int (*lock)(struct spi_dev_s *dev, bool lock);

    int (*select)(struct spi_dev_s *dev, uint32_t devid, bool selected);

    int (*exchange)(struct spi_dev_s *dev,
        const void *txbuffer, void *rxbuffer, size_t nwords);

    int (*exchangeblock)(struct spi_dev_s *dev,
        const void *txbuffer, void *rxbuffer, size_t nwords);

    int (*sndblock)(struct spi_dev_s *dev, const void *buffer, size_t nwords);

    int (*recvblock)(struct spi_dev_s *dev, void *buffer, size_t nwords);
};

struct spi_dev_s
{
    uint32_t frequency;
    enum spi_mode_e mode;
    uint8_t nbits;

#if defined(CONFIG_BOARD_FREERTOS_ENABLE)
    SemaphoreHandle_t  rxsem;    /* Wait for RX DMA to complete */
    SemaphoreHandle_t  txsem;    /* Wait for TX DMA to complete */

    /* exclsem handle by SPI_LOCK() */
    SemaphoreHandle_t  exclsem;  /* Held while chip is selected for mutual exclusion */
#else
    volatile uint32_t flag_tx;
    volatile uint32_t flag_rx;

    volatile uint32_t flag_excl;
#endif

    volatile uint8_t rxresult;     /* Result of the RX DMA */
    volatile uint8_t txresult;     /* Result of the TX DMA */

    /* Driver interface */
    const struct spi_ops_s *ops;  /* Arch-specific operations */
    void                   *priv; /* Used by the arch-specific logic */
};

#if defined(__cplusplus)
extern "C"{
#endif

int               spi_register(struct spi_dev_s *spi, int bus);
struct spi_dev_s* spi_bus_get(int bus);
int               spi_bus_initialize(int bus);

int  spi_devlock(struct spi_dev_s *dev, bool lock);
int  spi_dmarxwait(struct spi_dev_s *dev);
int  spi_dmatxwait(struct spi_dev_s *dev);
void spi_dmarxwakeup(struct spi_dev_s *dev);
void spi_dmatxwakeup(struct spi_dev_s *dev);

#if defined(__cplusplus)
}
#endif

#endif
