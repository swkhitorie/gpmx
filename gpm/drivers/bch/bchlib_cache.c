/****************************************************************************
 * drivers/bch/bchlib_cache.c
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
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <debug.h>

#include "bch.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: bchlib_flushsector
 *
 * Description:
 *   Flush the current contents of the sector buffer (if dirty)
 *
 * Assumptions:
 *   Caller must assume mutual exclusion
 *
 ****************************************************************************/

int bchlib_flushsector(struct bchlib_s *bch)
{
    struct inode *inode;
    ssize_t ret = 0; // OK

    /* Check if the sector has been modified and is out of synch with the
    * media.
    */

    if (bch->dirty) {
        inode = bch->inode;

        /* Write the sector to the media */

        ret = inode->u.i_bops->write(inode, bch->buffer, bch->sector, 1);

        if (ret < 0) {

            ferr("Write failed: %zd\n", ret);
            return (int)ret;
        }

        /* The sector is now in sync with the media */
        bch->dirty = false;
    }

    return (int)ret;
}

/****************************************************************************
 * Name: bchlib_readsector
 *
 * Description:
 *   Flush the current contents of the sector buffer (if dirty)
 *
 * Assumptions:
 *   Caller must assume mutual exclusion
 *
 ****************************************************************************/

int bchlib_readsector(struct bchlib_s *bch, size_t sector)
{
    struct inode *inode;
    ssize_t ret = 0; // OK

    if (bch->sector != sector) {
        inode = bch->inode;

        ret = bchlib_flushsector(bch);

        if (ret < 0) {

            ferr("Flush failed: %zd\n", ret);
            return (int)ret;
        }

        bch->sector = (size_t)-1;

        ret = inode->u.i_bops->read(inode, bch->buffer, sector, 1);

        if (ret < 0) {

            ferr("Read failed: %zd\n", ret);
            return (int)ret;
        }

        bch->sector = sector;
    }

    return (int)ret;
}
