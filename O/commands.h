/*
 * Created by Tomas Linhart on 4. 2. 2021.
 */

#ifndef SP_COMMANDS_H
#define SP_COMMANDS_H

#include "structures.h"
#include <stdio.h>
#include <stdlib.h>
#include "superblock.h"

int process_command_line(VFS **vfs, char *input);

void help();
void format(VFS **vfs, int32_t size);
void debug(VFS **vfs);
void incp(VFS** vfs, char *filepath_src, char *filepath_dest);
void outcp(VFS **vfs, char *filepath_src, char *filepath_dest);
void vfs_mkdir(VFS** vfs, char *dir_name);
void cd(VFS** vfs, char *path);
void ls(VFS** vfs, char *path);
void cat(VFS** vfs, char *file);
void pwd(VFS** vfs);
void info(VFS** vfs, char *path);
void rm(VFS **vfs, char *file);
void vfs_rmdir(VFS **vfs, char *path);
void cp(VFS **vfs, char *src_file, char *dest_file);
void mv(VFS **vfs, char *src_file, char *dest_file);
void load(VFS **vfs, char *filename);
void check(VFS **vfs);

void size(VFS **vfs, int32_t inode_id, int32_t new_size);

#endif //SP_COMMANDS_H
