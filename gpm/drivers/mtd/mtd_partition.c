/****************************************************************************
 * drivers/mtd/mtd_partition.c
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

/****************************************************************************
 * drivers/mtd/mtd_partition.c
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <gpmx/config.h>

#include <sys/types.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <debug.h>

#include <string.h>
#include <fcntl.h>

#include <driver/drv_sched.h>
#include <gpm/mtd/mtd.h>
#include <gpm/fs/ioctl.h>
#include <gpm/fs/fs.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define PART_NAME_MAX           15

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* This type represents the state of the MTD device.  The struct mtd_dev_s
 * must appear at the beginning of the definition so that you can freely
 * cast between pointers to struct mtd_dev_s and struct mtd_partition_s.
 */

struct mtd_partition_s
{
    /* This structure must reside at the beginning so that we can simply cast
    * from struct mtd_dev_s * to struct mtd_partition_s *
    */

    struct mtd_dev_s child;     /* The "child" MTD vtable that manages the
                                    * sub-region */

    /* Other implementation specific data may follow here */

    struct mtd_dev_s *parent;   /* The "parent" MTD driver that manages the
                                    * entire FLASH */
    off_t firstblock;           /* Offset to the first block of the managed
                                    * sub-region */
    off_t neraseblocks;         /* The number of erase blocks in the managed
                                    * sub-region */
    off_t blocksize;            /* The size of one read/write block */
    uint16_t blkpererase;       /* Number of R/W blocks in one erase block */

#ifdef CONFIG_MTD_PARTITION_NAMES
  char name[PART_NAME_MAX + 1]; /* Name of the partition */
#endif
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/* MTD driver methods */

static int     part_erase(struct mtd_dev_s *dev, off_t startblock,
                size_t nblocks);
static ssize_t part_bread(struct mtd_dev_s *dev, off_t startblock,
                size_t nblocks, uint8_t *buf);
static ssize_t part_bwrite(struct mtd_dev_s *dev, off_t startblock,
                size_t nblocks, const uint8_t *buf);
static ssize_t part_read(struct mtd_dev_s *dev, off_t offset,
                size_t nbytes, uint8_t *buffer);
#ifdef CONFIG_MTD_BYTE_WRITE
static ssize_t part_write(struct mtd_dev_s *dev, off_t offset,
                size_t nbytes, const uint8_t *buffer);
#endif
static int     part_ioctl(struct mtd_dev_s *dev, int cmd,
                unsigned long arg);

/****************************************************************************
 * Name: part_blockcheck
 *
 * Description:
 *   Check if the provided block offset lies within the partition
 *
 ****************************************************************************/

static bool part_blockcheck(struct mtd_partition_s *priv, off_t block)
{
    off_t partsize;

    partsize = priv->neraseblocks * priv->blkpererase;
    return block < partsize;
}

/****************************************************************************
 * Name: part_bytecheck
 *
 * Description:
 *   Check if the provided byte offset lies within the partition
 *
 ****************************************************************************/

static bool part_bytecheck(struct mtd_partition_s *priv, off_t byoff)
{
    off_t erasesize;
    off_t readend;

    erasesize = priv->blocksize * priv->blkpererase;
    readend   = (byoff + erasesize - 1) / erasesize;
    return readend <= priv->neraseblocks;
}

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: part_erase
 *
 * Description:
 *   Erase several blocks, each of the size previously reported.
 *
 ****************************************************************************/

static int part_erase(struct mtd_dev_s *dev, off_t startblock,
                    size_t nblocks)
{
    struct mtd_partition_s *priv = (struct mtd_partition_s *)dev;
    off_t eoffset;

    DEBUGASSERT(priv);

    /* Make sure that erase would not extend past the end of the partition */

    if (!part_blockcheck(priv, (startblock + nblocks - 1) * priv->blkpererase)) {
        ferr("ERROR: Erase beyond the end of the partition\n");
        return -ENXIO;
    }

    /* Just add the partition offset to the requested block and let the
    * underlying MTD driver perform the erase.
    *
    * NOTE: the offset here is in units of erase blocks.
    */

    eoffset = priv->firstblock / priv->blkpererase;
    DEBUGASSERT(eoffset * priv->blkpererase == priv->firstblock);

    return priv->parent->erase(priv->parent, startblock + eoffset, nblocks);
}

/****************************************************************************
 * Name: part_bread
 *
 * Description:
 *   Read the specified number of blocks into the user provided buffer.
 *
 ****************************************************************************/

static ssize_t part_bread(struct mtd_dev_s *dev, off_t startblock,
                          size_t nblocks, uint8_t *buf)
{
    struct mtd_partition_s *priv = (struct mtd_partition_s *)dev;

    DEBUGASSERT(priv && (buf || nblocks == 0));

    /* Make sure that read would not extend past the end of the partition */

    if (!part_blockcheck(priv, startblock + nblocks - 1)) {
        ferr("ERROR: Read beyond the end of the partition\n");
        return -ENXIO;
    }

    /* Just add the partition offset to the requested block and let the
    * underlying MTD driver perform the read.
    */

    return priv->parent->bread(priv->parent, startblock + priv->firstblock,
                                nblocks, buf);
}

/****************************************************************************
 * Name: part_bwrite
 *
 * Description:
 *   Write the specified number of blocks from the user provided buffer.
 *
 ****************************************************************************/

static ssize_t part_bwrite(struct mtd_dev_s *dev, off_t startblock,
                           size_t nblocks, const uint8_t *buf)
{
    struct mtd_partition_s *priv = (struct mtd_partition_s *)dev;

    DEBUGASSERT(priv && (buf || nblocks == 0));

    /* Make sure that write would not extend past the end of the partition */

    if (!part_blockcheck(priv, startblock + nblocks - 1)) {
        ferr("ERROR: Write beyond the end of the partition\n");
        return -ENXIO;
    }

    /* Just add the partition offset to the requested block and let the
    * underlying MTD driver perform the write.
    */

    return priv->parent->bwrite(priv->parent, startblock + priv->firstblock,
                                nblocks, buf);
}

/****************************************************************************
 * Name: part_read
 *
 * Description:
 *   Read the specified number of bytes to the user provided buffer.
 *
 ****************************************************************************/

static ssize_t part_read(struct mtd_dev_s *dev, off_t offset,
                         size_t nbytes, uint8_t *buffer)
{
    struct mtd_partition_s *priv = (struct mtd_partition_s *)dev;
    off_t newoffset;

    /* Does the underlying MTD device support the read method? */

    if (priv->parent->read) {

        /* Make sure that read would not extend past the end of the partition */

        if (!part_bytecheck(priv, offset + nbytes - 1)) {
            return -ENXIO;
        }

        /* Just add the partition offset to the requested block and let the
        * underlying MTD driver perform the read.
        */

        newoffset = offset + priv->firstblock * priv->blocksize;
        return priv->parent->read(priv->parent, newoffset, nbytes, buffer);
    }

    /* The underlying MTD driver does not support the read() method */
    return -ENOSYS;
}

/****************************************************************************
 * Name: part_write
 ****************************************************************************/

#ifdef CONFIG_MTD_BYTE_WRITE
static ssize_t part_write(struct mtd_dev_s *dev, off_t offset,
                          size_t nbytes, const uint8_t *buffer)
{
    struct mtd_partition_s *priv = (struct mtd_partition_s *)dev;
    off_t newoffset;

    /* Does the underlying MTD device support the write method? */

    if (priv->parent->write) {

        /* Make sure that write would not extend past the end of the
        * partition
        */

        if (!part_bytecheck(priv, offset + nbytes - 1)) {

            return -ENXIO;
        }

        /* Just add the partition offset to the requested block and let the
        * underlying MTD driver perform the write.
        */

        newoffset = offset + priv->firstblock * priv->blocksize;
        return priv->parent->write(priv->parent, newoffset, nbytes, buffer);
    }

    /* The underlying MTD driver does not support the write() method */
    return -ENOSYS;
}
#endif

/****************************************************************************
 * Name: part_ioctl
 ****************************************************************************/

static int part_ioctl(struct mtd_dev_s *dev, int cmd, unsigned long arg)
{
    struct mtd_partition_s *priv = (struct mtd_partition_s *)dev;
    int ret = -EINVAL; /* Assume good command with bad parameters */

    switch (cmd) {

    case MTDIOC_GEOMETRY:
        {
            struct mtd_geometry_s *geo = (struct mtd_geometry_s *)arg;
            if (geo) {

                /* Populate the geometry structure with information needed to
                * know the capacity and how to access the device.
                */

                geo->blocksize    = priv->blocksize;
                geo->erasesize    = priv->blocksize * priv->blkpererase;
                geo->neraseblocks = priv->neraseblocks;
                ret               = 0;
            }
        }
        break;

    case BIOC_PARTINFO:
        {
            struct partition_info_s *info =
                (struct partition_info_s *)arg;

            if (info != NULL) {
                info->numsectors  = priv->neraseblocks * priv->blkpererase;
                info->sectorsize  = priv->blocksize;
                info->startsector = priv->firstblock;

                strncpy(info->parent, priv->parent->name, NAME_MAX);

                ret = 0;
            }
        }
        break;

    case BIOC_XIPBASE:
        {
            void **ppv = (void**)arg;
            unsigned long base;

            if (ppv) {

                /* Get the XIP base of the entire FLASH */

                ret = priv->parent->ioctl(priv->parent, BIOC_XIPBASE,
                                            (unsigned long)((uintptr_t)&base));

                if (ret == 0) {
                    /* Add the offset of this partition to the XIP base and
                    * return the sum to the caller.
                    */

                    *ppv = (void *)(uintptr_t)
                                (base + priv->firstblock * priv->blocksize);
                }
            }
        }
        break;

    case MTDIOC_BULKERASE:
        {
            /* Erase the entire partition */

            ret = priv->parent->erase(priv->parent,
                                        priv->firstblock / priv->blkpererase,
                                        priv->neraseblocks);
        }
        break;

    default:
        {
            /* Pass any unhandled ioctl() calls to the underlying driver */

            ret = priv->parent->ioctl(priv->parent, cmd, arg);
        }
        break;
    }

    return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: mtd_partition
 *
 * Description:
 *   Give an instance of an MTD driver, create a flash partition, ie.,
 *   another MTD driver instance that only operates with a sub-region of
 *   FLASH media.  That sub-region is defined by a sector offset and a
 *   sector count (where the size of a sector is provided the by parent MTD
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
                                    off_t firstblock,
                                    off_t nblocks)
{
    struct mtd_partition_s *part;
    struct mtd_geometry_s geo;
    unsigned int blkpererase;
    off_t erasestart;
    off_t eraseend;
    int ret;

    /* Get the geometry of the FLASH device */

    ret = mtd->ioctl(mtd, MTDIOC_GEOMETRY, (unsigned long)((uintptr_t)&geo));
    if (ret < 0) {
        return NULL;
    }

    /* Get the number of blocks per erase.  There must be an even number of
    * blocks in one erase blocks.
    */

    blkpererase = geo.erasesize / geo.blocksize;

    /* Adjust the offset and size if necessary so that they are multiples of
    * the erase block size (making sure that we do not go outside of the
    * requested sub-region).  NOTE that 'eraseend' is the first erase block
    * beyond the sub-region.
    */

    erasestart = (firstblock + blkpererase - 1) / blkpererase;
    eraseend   = (firstblock + nblocks) / blkpererase;

    if (erasestart >= eraseend) {
        return NULL;
    }

    /* Verify that the sub-region is valid for this geometry */

    if (eraseend > geo.neraseblocks) {
        return NULL;
    }

    /* Allocate a partition device structure */

    part = (struct mtd_partition_s *)
            kmm_zalloc(sizeof(struct mtd_partition_s));

    if (!part) {
        return NULL;
    }

    /* Initialize the partition device structure. (unsupported methods were
    * nullified by kmm_zalloc).
    */

    part->child.erase  = part_erase;
    part->child.bread  = part_bread;
    part->child.bwrite = part_bwrite;
    part->child.read   = mtd->read ? part_read : NULL;
    part->child.ioctl  = part_ioctl;
#ifdef CONFIG_MTD_BYTE_WRITE
    part->child.write  = mtd->write ? part_write : NULL;
#endif
    part->child.name   = "part";

    part->parent       = mtd;
    part->firstblock   = erasestart * blkpererase;
    part->neraseblocks = eraseend - erasestart;
    part->blocksize    = geo.blocksize;
    part->blkpererase  = blkpererase;

#ifdef CONFIG_MTD_PARTITION_NAMES
    strcpy(part->name, "(noname)");
#endif

    /* Return the implementation-specific state structure as the MTD device */

    return &part->child;
}

/****************************************************************************
 * Name: mtd_setpartitionname
 *
 * Description:
 *   Sets the name of the specified partition.
 *
 ****************************************************************************/

#ifdef CONFIG_MTD_PARTITION_NAMES
int mtd_setpartitionname(struct mtd_dev_s *mtd, const char *name)
{
    struct mtd_partition_s *priv = (struct mtd_partition_s *)mtd;

    if (priv == NULL || name == NULL) {

        return -EINVAL;
    }

    /* Allocate space for the name */
    strlcpy(priv->name, name, sizeof(priv->name));
    return 0;
}
#endif
