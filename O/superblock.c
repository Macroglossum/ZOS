/*
 * Created by Tomas on 4. 2. 2021.
 */

#include "superblock.h"

/*
 * Creates new superblock and loads default values
 */
superblock *superblock_init(int32_t disk_size){
    superblock *sblock = calloc(1, sizeof(superblock));
    strcpy(sblock->signature, SUPERBLOCK_SIGNATURE);

    sblock->disk_size = disk_size;
    sblock->cluster_size = CLUSTER_SIZE;
    sblock->cluster_count = disk_size / CLUSTER_SIZE;							                                            /* Number of clusters in VFS */
    sblock->inode_cluster_count = sblock->cluster_count / 20;                                                               /* Number of block reserved for inodes (5%) */
    sblock->inode_count = (sblock->inode_cluster_count * CLUSTER_SIZE) / sizeof(inode);	                                    /* Number of inodes */
    sblock->bitmap_cluster_count = ceil((sblock->cluster_count - sblock->inode_cluster_count - 1) / (float)CLUSTER_SIZE);
    sblock->data_cluster_count = sblock->cluster_count - 1 - sblock->bitmap_cluster_count - sblock->inode_cluster_count;    /* Number of data blocks */
    sblock->bitmap_start_address = CLUSTER_SIZE;                                                                            /* Bitmap start address - First Cluster is reserved for superblock */
    sblock->inode_start_address = sblock->bitmap_start_address + CLUSTER_SIZE * sblock->bitmap_cluster_count;				/* Bitmap address + Number of bitmap blocks * Block size */
    sblock->data_start_address = sblock->inode_start_address + CLUSTER_SIZE * sblock->inode_cluster_count;                  /* Inode array address + Number of inodes blocks * block size */
    return sblock;
}

