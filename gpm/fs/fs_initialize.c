#include "inode/inode.h"

void fs_initialize(void)
{
    /* Initial inode, file, and VFS data structures */
    inode_initialize();
}
