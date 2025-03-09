#ifndef DEV_OPS_NODE_H_
#define DEV_OPS_NODE_H_


struct dnode
{
    char name[16];

    struct dnode *i_peer;     /* Link to same level inode */
    struct dnode *i_child;    /* Link to lower level inode */
    union inode_ops_u u;          /* Inode operations */
    void         *i_private;  /* Per inode driver private data */
};

#endif
