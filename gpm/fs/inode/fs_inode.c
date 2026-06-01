#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include "gpm/fs/fs.h"
#include "gpm/mutex.h"
#include "inode/inode.h"

static rmutex_t g_inode_lock;

void inode_initialize(void)
{
  krmutex_init(&g_inode_lock);
  /* Reserve the root node */
  inode_root_reserve();
}

int inode_semtake(void)
{
  return krmutex_lock(&g_inode_lock);
}

void inode_semgive(void)
{
  krmutex_unlock(&g_inode_lock);
}
