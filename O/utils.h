/*
 * Created by Tomas Linhart on 4. 2. 2021.
 */

#ifndef SP_UTILS_H
#define SP_UTILS_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "structures.h"

bool streq(char *str1, char *str2);
char * get_line();
bool file_exists (char *filename);
char *remove_nl (char * message);
int32_t get_size_from_string(char *string_size);
bool strempty (char *str);
int parse_path(VFS **vfs, char *path, char **name, directory **dir);
dir_item *find_item(dir_item *first_item, char *name);
bool verify_format(VFS **vfs);
void prepend(char* s, const char* t);
void print_dir_item_info(VFS** vfs, dir_item *item);
int get_block_count_with_indirect(int block_count);
int get_last_block_size(int rest);
int32_t convert_int(char *str);

#endif //SP_UTILS_H
