#ifndef __INCLUDE_MTD_H
#define __INCLUDE_MTD_H

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

#include "gpm/fs/ioctl.h"

/* Ioctl commands */

/* Note, the following ioctl existed in the past and was removed:
 * #define MTDIOC_XIPBASE    _MTDIOC(0x0002)
 * #define MTDIOC_FLUSH      _MTDIOC(0x0009)
 * #define MTDIOC_PARTINFO   _MTDIOC(0x000b)
 * try to avoid adding a new ioctl with the same ioctl number and
 * replace with BIOC_XIPBASE, BIOC_FLUSH and BIOC_PARTINFO instead.
 */

#define MTDIOC_GEOMETRY   _MTDIOC(0x0001) /* IN:  Pointer to write-able struct
                                           *      mtd_geometry_s in which to receive
                                           *      receive geometry data (see mtd.h)
                                           * OUT: Geometry structure is populated
                                           *      with data for the MTD */
#define MTDIOC_BULKERASE  _MTDIOC(0x0003) /* IN:  None
                                           * OUT: None */
#define MTDIOC_PROTECT    _MTDIOC(0x0004) /* IN:  Pointer to read-able struct
                                           *      mtd_protects_s that provides
                                           *      the region to protect.
                                           * OUT: None */
#define MTDIOC_UNPROTECT  _MTDIOC(0x0005) /* IN:  Pointer to read-able struct
                                           *      mtd_protects_s that provides
                                           *      the region to un-protect.
                                           * OUT: None */
#define MTDIOC_SETSPEED   _MTDIOC(0x0006) /* IN:  New bus speed in Hz
                                           * OUT: None */
#define MTDIOC_EXTENDED   _MTDIOC(0x0007) /* IN:  unsigned long
                                           *      0=Use normal memory region
                                           *      1=Use alternate/extended memory
                                           * OUT: None */
#define MTDIOC_ECCSTATUS  _MTDIOC(0x0008) /* IN:  Pointer to uint8_t
                                           * OUT: ECC status */
#define MTDIOC_ERASESTATE _MTDIOC(0x000a) /* IN:  Pointer to uint8_t
                                           * OUT: Byte value that represents the
                                           *      erased state of the MTD cell */

/* Macros to hide implementation */

#define MTD_ERASE(d,s,n)   ((d)->erase   ? (d)->erase(d,s,n)    : (-ENOSYS))
#define MTD_BREAD(d,s,n,b) ((d)->bread   ? (d)->bread(d,s,n,b)  : (-ENOSYS))
#define MTD_BWRITE(d,s,n,b)((d)->bwrite  ? (d)->bwrite(d,s,n,b) : (-ENOSYS))
#define MTD_READ(d,s,n,b)  ((d)->read    ? (d)->read(d,s,n,b)   : (-ENOSYS))
#define MTD_WRITE(d,s,n,b) ((d)->write   ? (d)->write(d,s,n,b)  : (-ENOSYS))
#define MTD_IOCTL(d,c,a)   ((d)->ioctl   ? (d)->ioctl(d,c,a)    : (-ENOSYS))

struct qspi_dev_s;    /* Forward reference */

/* The following defines the geometry for the device.  It treats the device
 * as though it were just an array of fixed size blocks.  That is most likely
 * not true, but the client will expect the device logic to do whatever is
 * necessary to make it appear so.
 */

struct mtd_geometry_s
{
  uint32_t blocksize;     /* Size of one read/write block. */
  uint32_t erasesize;     /* Size of one erase blocks -- must be a multiple
                           * of blocksize. */
  uint32_t neraseblocks;  /* Number of erase blocks */
};

/* This structure describes a range of sectors to be protected or
 * unprotected.
 */

struct mtd_protect_s
{
  off_t  startblock;      /* First block to be [un-]protected */
  size_t nblocks;         /* Number of blocks to [un-]protect */
};

/* The following defines the information for writing bytes to a sector
 * that are not a full page write (bytewrite).
 */

struct mtd_byte_write_s
{
  uint32_t offset;        /* Offset within the device to write to */
  uint16_t count;         /* Number of bytes to write */
  const uint8_t *buffer;  /* Pointer to the data to write */
};

/* This structure defines the interface to a simple memory technology device.
 * It will likely need to be extended in the future to support more complex
 * devices.
 */

struct mtd_dev_s
{
  /* The following methods operate on the MTD: */

  /* Erase the specified erase blocks (units are erase blocks).  Semantic
   * Clarification:  Here, we are not referring to the erase block according
   * to the FLASH data sheet.  Rather, we are referring to the *smallest*
   * erasable part of the FLASH which may have a name like a page or sector
   * or subsector.
   */

  int (*erase)(struct mtd_dev_s *dev, off_t startblock, size_t nblocks);

  /* Read/write from the specified read/write blocks */

  ssize_t (*bread)(struct mtd_dev_s *dev, off_t startblock,
                   size_t nblocks, uint8_t *buffer);
  ssize_t (*bwrite)(struct mtd_dev_s *dev, off_t startblock,
                    size_t nblocks, const uint8_t *buffer);

  /* Some devices may support byte oriented reads (optional).  Most MTD
   * devices are inherently block oriented so byte-oriented writing is not
   * supported. It is recommended that low-level drivers not support read()
   * if it requires buffering.
   */

  ssize_t (*read)(struct mtd_dev_s *dev, off_t offset, size_t nbytes,
                  uint8_t *buffer);

  ssize_t (*write)(struct mtd_dev_s *dev, off_t offset, size_t nbytes,
                   const uint8_t *buffer);

  /* Support other, less frequently used commands:
   *  - MTDIOC_GEOMETRY:  Get MTD geometry
   *  - MTDIOC_XIPBASE:   Convert block to physical address for
   *    eXecute-In-Place
   *  - MTDIOC_BULKERASE: Erase the entire device
   */

  int (*ioctl)(struct mtd_dev_s *dev, int cmd, unsigned long arg);

  /* Name of this MTD device */

  const char *name;
};

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

struct mtd_dev_s *mtd_partition(struct mtd_dev_s *mtd,
                                    off_t firstblock, off_t nblocks);

int ftl_initialize_by_path(const char *path, struct mtd_dev_s *mtd);

int ftl_initialize(int minor, struct mtd_dev_s *mtd);

int smart_initialize(int minor, struct mtd_dev_s *mtd,
                     const char *partname);

/* MTD Driver Initialization ************************************************/

struct mtd_dev_s *progmem_initialize(void);

/****************************************************************************
 * Description:
 *   Initializes the driver for SPI-based W25x16, x32, and x64 and W25q16,
 *   q32, q64, and q128 FLASH
 ****************************************************************************/
struct spi_dev_s;
struct mtd_dev_s *w25_initialize(struct spi_dev_s *dev);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif
