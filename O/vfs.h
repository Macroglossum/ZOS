//
// Created by Tomas on 4. 2. 2021.
//

#ifndef SP_VFS_H
#define SP_VFS_H

#include "structures.h"
#include <stdlib.h>


void initialize_vfs(VFS **vfs, char *vfs_name);
directory *find_directory(VFS** vfs, char *path);
int32_t *find_free_data_blocks(VFS** vfs, int count);
int32_t find_free_inode(VFS** vfs);
bool vfs_exists(directory *dir, char *name);
void update_bitmap_in_file(VFS** vfs, dir_item *item, int8_t value, int32_t *data_blocks, int b_count);
int update_directory_in_file(VFS** vfs, directory *dir, dir_item *item, bool create);
void update_sizes_in_file(VFS** vfs, directory *dir, int32_t size);
int32_t *get_data_blocks(VFS** vfs, int32_t nodeid, int *block_count, int *rest);
int initialize_inode(VFS** vfs, int32_t inode_id, int32_t size, int block_count, int32_t *blocks);
void write_inode_to_vfs(VFS **vfs, int id);

void flush_vfs(VFS **vfs);
int seek_data_cluster(VFS **vfs, int block_number);
int seek_set(VFS **vfs, long int offset);
int seek_cur(VFS **vfs, long int offset);
int seek_end(VFS **vfs, long int offset);
size_t write_vfs(VFS **vfs, const void * ptr, size_t size, size_t count);
size_t int8_write_vfs(VFS **vfs, const void * ptr);
size_t int32_write_vfs(VFS **vfs, const void * ptr);
size_t read_vfs(VFS **vfs, void * ptr, size_t size, size_t count);
size_t int8_read_vfs ( VFS **vfs, void * ptr);
size_t int32_read_vfs ( VFS **vfs, void * ptr);
void rewind_vfs(VFS **vfs);

#endif //SP_VFS_H
