/****************************************************************************
 * include/gpm/mtd/mtd.h
 * Memory Technology Device (MTD) interface
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef __INCLUDE_GPM_MTD_MTD_H
#define __INCLUDE_GPM_MTD_MTD_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <gpmx/config.h>

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

#include <gpm/fs/ioctl.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

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

#ifdef CONFIG_MTD_BYTE_WRITE
    ssize_t (*write)(struct mtd_dev_s *dev, off_t offset, size_t nbytes,
                    const uint8_t *buffer);
#endif

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

/* MTD Support **************************************************************/

/****************************************************************************
 * Name: mtd_partition
 *
 * Description:
 *   Given an instance of an MTD driver, create a flash partition, ie.,
 *   another MTD driver instance that only operates with a sub-region of
 *   FLASH media.  That sub-region is defined by a sector offset and a
 *   sector count (where the size of a sector is provided by the parent MTD
 *   driver).
 *
 *   NOTE: Since there may be a number of MTD partition drivers operating on
 *   the same, underlying FLASH driver, that FLASH driver must be capable
 *   of enforcing mutually exclusive access to the FLASH device.  Without
 *   partitions, that mutual exclusion would be provided by the file system
 *   above the FLASH driver.
 *
 * Input Parameters:
 *   mtd        - The MTD device to be partitioned
 *   firstblock - The offset in bytes to the first block
 *   nblocks    - The number of blocks in the partition
 *
 * Returned Value:
 *   On success, another MTD device representing the partition is returned.
 *   A NULL value is returned on a failure.
 *
 ****************************************************************************/

struct mtd_dev_s *mtd_partition(struct mtd_dev_s *mtd,
                                    off_t firstblock, off_t nblocks);

/****************************************************************************
 * Name: mtd_setpartitionname
 *
 * Description:
 *   Sets the name of the specified partition.
 *
 ****************************************************************************/

#ifdef CONFIG_MTD_PARTITION_NAMES
int mtd_setpartitionname(struct mtd_dev_s *mtd, const char *name);
#endif

/****************************************************************************
 * Name: ftl_initialize_by_path
 *
 * Description:
 *   Initialize to provide a block driver wrapper around an MTD interface
 *
 * Input Parameters:
 *   path - The block device path.
 *   mtd  - The MTD device that supports the FLASH interface.
 *
 ****************************************************************************/

int ftl_initialize_by_path(const char *path, struct mtd_dev_s *mtd);

/****************************************************************************
 * Name: ftl_initialize
 *
 * Description:
 *   Initialize to provide a block driver wrapper around an MTD interface
 *
 * Input Parameters:
 *   minor - The minor device number.  The MTD block device will be
 *      registered as as /dev/mtdblockN where N is the minor number.
 *   mtd - The MTD device that supports the FLASH interface.
 *
 ****************************************************************************/

int ftl_initialize(int minor, struct mtd_dev_s *mtd);

/* MTD Driver Initialization ************************************************/

/* Create an initialized MTD device instance for a particular memory device.
 * MTD devices are not registered in the file system as are other device
 * driver but, but are created as instances that can be bound to other
 * functions (such as a block or character driver front end).
 */

/****************************************************************************
 * Name: at24c_initialize
 *
 * Description:
 *   Initializes the driver for I2C-based at24cxx EEPROM(AT24C32, AT24C64,
 *   AT24C128, AT24C256)
 *
 ****************************************************************************/

struct i2c_master_s; /* Forward reference */

#ifdef CONFIG_AT24XX_MULTI
struct mtd_dev_s *at24c_initialize(struct i2c_master_s *dev,
                                  uint8_t address);
#else
struct mtd_dev_s *at24c_initialize(struct i2c_master_s *dev);
#endif

/****************************************************************************
 * Name: w25_initialize
 *
 * Description:
 *   Initializes the driver for SPI-based W25x16, x32, and x64 and W25q16,
 *   q32, q64, and q128 FLASH
 *
 ****************************************************************************/

struct spi_dev_s;
struct mtd_dev_s *w25_initialize(struct spi_dev_s *dev);

/****************************************************************************
 * Name: ramtron_initialize
 *
 * Description:
 *   Create and initialize a Ramtron MTD device instance.
 *
 * Input Parameters:
 *   start - Address of the beginning of the allocated RAM regions.
 *   size  - The size in bytes of the allocated RAM region.
 *
 ****************************************************************************/

struct mtd_dev_s *ramtron_initialize(struct spi_dev_s *dev);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* __INCLUDE_GPM_MTD_MTD_H */
