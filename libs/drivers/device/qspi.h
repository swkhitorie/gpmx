
#ifndef DEV_OPS_QUAD_SPI_H_
#define DEV_OPS_QUAD_SPI_H_

#ifdef CONFIG_GPDRIVE_QUADSPI

#include "dnode.h"

/****************************************************************************
 * Name: QSPI_LOCK
 *
 * Description:
 *   On QSPI busses where there are multiple devices, it will be necessary to
 *   lock QSPI to have exclusive access to the busses for a sequence of
 *   transfers.  The bus should be locked before the chip is selected. After
 *   locking the QSPI bus, the caller should then also call the setfrequency,
 *   setbits, and setmode methods to make sure that the QSPI is properly
 *   configured for the device.  If the QSPI buss is being shared, then it
 *   may have been left in an incompatible state.
 *
 * Input Parameters:
 *   dev  - Device-specific state data
 *   lock - true: Lock qspi bus, false: unlock QSPI bus
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

#define QSPI_LOCK(d,l) (d)->ops->lock(d,l)

/****************************************************************************
 * Name: QSPI_SETFREQUENCY
 *
 * Description:
 *   Set the QSPI frequency. Required.
 *
 * Input Parameters:
 *   dev -       Device-specific state data
 *   frequency - The QSPI frequency requested
 *
 * Returned Value:
 *   Returns the actual frequency selected
 *
 ****************************************************************************/

#define QSPI_SETFREQUENCY(d,f) ((d)->ops->setfrequency(d,f))

/****************************************************************************
 * Name: QSPI_SETMODE
 *
 * Description:
 *   Set the QSPI mode. Optional.  See enum qspi_mode_e for mode definitions.
 *
 * Input Parameters:
 *   dev -  Device-specific state data
 *   mode - The QSPI mode requested
 *
 * Returned Value:
 *   none
 *
 ****************************************************************************/

#define QSPI_SETMODE(d,m) (d)->ops->setmode(d,m)

/****************************************************************************
 * Name: QSPI_SETBITS
 *
 * Description:
 *   Set the number if bits per word.
 *
 * Input Parameters:
 *   dev -  Device-specific state data
 *   nbits - The number of bits requests.
 *           If value is greater > 0 then it implies MSB first
 *           If value is below < 0, then it implies LSB first with -nbits
 *
 * Returned Value:
 *   none
 *
 ****************************************************************************/

#define QSPI_SETBITS(d,b) (d)->ops->setbits(d,b)

/****************************************************************************
 * Name: QSPI_COMMAND
 *
 * Description:
 *   Perform one QSPI command transfer
 *
 * Input Parameters:
 *   dev     - Device-specific state data
 *   cmdinfo - Describes the command transfer to be performed.
 *
 * Returned Value:
 *   Zero (OK) on SUCCESS, a negated errno on value of failure
 *
 ****************************************************************************/

#define QSPI_COMMAND(d,c) (d)->ops->command(d,c)

/* QSPI Command Transfer Flags */

#define QSPICMD_ADDRESS       (1 << 0)  /* Bit 0: Enable address transfer */
#define QSPICMD_READDATA      (1 << 1)  /* Bit 1: Enable read data transfer */
#define QSPICMD_WRITEDATA     (1 << 2)  /* Bit 2: Enable write data transfer */
#define QSPICMD_IDUAL         (1 << 3)  /* Bit 3: Instruction on two lines */
#define QSPICMD_IQUAD         (1 << 4)  /* Bit 4: Instruction on four lines */

#define QSPICMD_ISADDRESS(f)  (((f) & QSPICMD_ADDRESS) != 0)
#define QSPICMD_ISDATA(f)     (((f) & (QSPICMD_READDATA | QSPICMD_WRITEDATA)) != 0)
#define QSPICMD_ISREAD(f)     (((f) & QSPICMD_READDATA) != 0)
#define QSPICMD_ISWRITE(f)    (((f) & QSPICMD_WRITEDATA) != 0)
#define QSPICMD_ISIDUAL(f)    (((f) & QSPICMD_IDUAL) != 0)
#define QSPICMD_ISIQUAD(f)    (((f) & QSPICMD_IQUAD) != 0)

/****************************************************************************
 * Name: QSPI_MEMORY
 *
 * Description:
 *   Perform one QSPI memory transfer
 *
 * Input Parameters:
 *   dev     - Device-specific state data
 *   meminfo - Describes the memory transfer to be performed.
 *
 * Returned Value:
 *   Zero (OK) on SUCCESS, a negated errno on value of failure
 *
 ****************************************************************************/

#define QSPI_MEMORY(d,m) (d)->ops->memory(d,m)

/* QSPI Memory Transfer Flags */

#define QSPIMEM_READ          (0)       /* Bit 2: 0=Memory read data transfer  */
#define QSPIMEM_WRITE         (1 << 2)  /* Bit 2: 1=Memory write data transfer */
#define QSPIMEM_DUALIO        (1 << 3)  /* Bit 3: Use Dual I/O (READ only)     */
#define QSPIMEM_QUADIO        (1 << 4)  /* Bit 4: Use Quad I/O (READ only)     */
#define QSPIMEM_SCRAMBLE      (1 << 5)  /* Bit 5: Scramble data                */
#define QSPIMEM_RANDOM        (1 << 6)  /* Bit 6: Use random key in scrambler  */
#define QSPIMEM_IDUAL         (1 << 7)  /* Bit 7: Instruction on two lines     */
#define QSPIMEM_IQUAD         (1 << 0)  /* Bit 0: Instruction on four lines    */

#define QSPIMEM_ISREAD(f)     (((f) & QSPIMEM_WRITE) == 0)
#define QSPIMEM_ISWRITE(f)    (((f) & QSPIMEM_WRITE) != 0)
#define QSPIMEM_ISDUALIO(f)   (((f) & QSPIMEM_DUALIO) != 0)
#define QSPIMEM_ISQUADIO(f)   (((f) & QSPIMEM_QUADIO) != 0)
#define QSPIMEM_ISSCRAMBLE(f) (((f) & QSPIMEM_SCRAMBLE) != 0)
#define QSPIMEM_ISIDUAL(f)    (((f) & QSPIMEM_IDUAL) != 0)
#define QSPIMEM_ISIQUAD(f)    (((f) & QSPIMEM_IQUAD) != 0)

#define QSPIMEM_ISRANDOM(f) \
  (((f) & (QSPIMEM_SCRAMBLE|QSPIMEM_RANDOM)) == \
          (QSPIMEM_SCRAMBLE|QSPIMEM_RANDOM))

/****************************************************************************
 * Name: QSPI_ALLOC
 *
 * Description:
 *   Allocate a buffer suitable for DMA data transfer
 *
 * Input Parameters:
 *   dev    - Device-specific state data
 *   buflen - Buffer length to allocate in bytes
 *
 * Returned Value:
 *   Address of tha allocated memory on success; NULL is returned on any
 *   failure.
 *
 ****************************************************************************/

#define QSPI_ALLOC(d,b) (d)->ops->alloc(d,b)

/****************************************************************************
 * Name: QSPI_FREE
 *
 * Description:
 *   Free memory returned by QSPI_ALLOC
 *
 * Input Parameters:
 *   dev    - Device-specific state data
 *   buffer - Buffer previously allocated via QSPI_ALLOC
 *
 * Returned Value:
 *   None.
 *
 ****************************************************************************/

#define QSPI_FREE(d,b) (d)->ops->free(d,b)

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Certain QSPI devices may required different clocking modes */

enum qspi_mode_e
{
  QSPIDEV_MODE0 = 0,     /* CPOL=0 CHPHA=0 */
  QSPIDEV_MODE1,         /* CPOL=0 CHPHA=1 */
  QSPIDEV_MODE2,         /* CPOL=1 CHPHA=0 */
  QSPIDEV_MODE3          /* CPOL=1 CHPHA=1 */
};

/* This structure describes one command transfer */

struct qspi_cmdinfo_s
{
  uint8_t   flags;       /* See QSPICMD_* definitions */
  uint8_t   addrlen;     /* Address length in bytes (if QSPICMD_ADDRESS) */
  uint16_t  cmd;         /* Command */
  uint16_t  buflen;      /* Data buffer length in bytes (if QSPICMD_DATA) */
  uint32_t  addr;        /* Address (if QSPICMD_ADDRESS) */
  void *buffer;          /* Data buffer (if QSPICMD_DATA) */
};

/* This structure describes one memory transfer */

struct qspi_meminfo_s
{
  uint8_t   flags;       /* See QSPIMEM_* definitions */
  uint8_t   addrlen;     /* Address length in bytes */
  uint8_t   dummies;     /* Number of dummy read cycles (READ only) */
  uint16_t  buflen;      /* Data buffer length in bytes */
  uint16_t  cmd;         /* Memory access command */
  uint32_t  addr;        /* Memory Address */
  uint32_t  key;         /* Scrambler key */
  void *buffer;          /* Data buffer */
};

struct qspi_msg_s
{
  uint8_t cmd;
  uint8_t cmd_lines;

  uint32_t addr;
  uint8_t addr_sz;
  uint8_t addr_lines;

  uint32_t alt_bytes;
  uint8_t alt_bytes_sz;
  uint8_t alt_bytes_lines;

  uint32_t dummies;

  uint8_t xfer_lines;
};

/* The QSPI vtable */

struct qspi_dev_s;
struct qspi_ops_s
{
  int       (*setup)(struct qspi_dev_s *dev);
  int       (*lock)(struct qspi_dev_s *dev, bool lock);
  uint32_t  (*setfrequency)(struct qspi_dev_s *dev,
                    uint32_t frequency);
  void      (*setmode)(struct qspi_dev_s *dev,
                    enum qspi_mode_e mode);
  void      (*setbits)(struct qspi_dev_s *dev, int nbits);
  int       (*command)(struct qspi_dev_s *dev,
                    struct qspi_cmdinfo_s *cmdinfo);
  int       (*memory)(struct qspi_dev_s *dev,
                    struct qspi_meminfo_s *meminfo);
  void     *(*alloc)(struct qspi_dev_s *dev, size_t buflen);
  void      (*free)(struct qspi_dev_s *dev, void *buffer);
};

/* QSPI private data.  This structure only defines the initial fields of the
 * structure visible to the QSPI client.  The specific implementation may
 * add additional, device specific fields
 */

struct qspi_dev_s
{

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

  const struct qspi_ops_s *ops;
  void                    *priv; /* Used by the arch-specific logic */
};

#if defined(__cplusplus)
extern "C" {
#endif

int                qspi_register(struct qspi_dev_s *qspi, int bus);
struct qspi_dev_s* qspi_bus_get(int bus);
int                qspi_bus_initialize(int bus);

int  qspi_devlock(struct qspi_dev_s *dev, bool lock);
int  qspi_txwait(struct qspi_dev_s *dev);
void qspi_txwakeup(struct qspi_dev_s *dev);
int  qspi_rxwait(struct qspi_dev_s *dev);
void qspi_rxwakeup(struct qspi_dev_s *dev);

#if defined(__cplusplus)
}
#endif

#endif // end with macro CONFIG_GPDRIVE_QUADSPI

#endif
