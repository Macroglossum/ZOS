//
// Created by Stepan Mocjak on 23.01.2021.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "fs.h"
#include "constants.h"

void set_variables(char *fs_file) {
    char *file_pointer = fs_file;

    fs = fs_file;

    if (!sb) {
        sb = fs_file;
    }

    file_pointer += sizeof(superblock);
    bitmap = file_pointer;
    file_pointer += sb->inode_start_address - sb->bitmap_start_address;
    inodes = file_pointer;
    file_pointer += sb->data_start_address - sb->inode_start_address;
    data_blocks = file_pointer;
    bitmap_size = sb->inode_start_address - sb->bitmap_start_address;
    inodes_count = (sb->data_start_address - sb->inode_start_address) / sizeof(inode);
}

char *read_file(char *file_name) {
    FILE *file;
    char *fs_file;
    long size, bytes_read;

    file = fopen(file_name, "rb");

    if(!file) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);

    fs_file = calloc(size + 1, sizeof(char));

    if(!fs_file) {
        return NULL;
    }

    bytes_read = fread(fs_file, size, 1, file);

    if(!bytes_read) {
        free(fs_file);
        fclose(file);
        return NULL;
    }

    fclose(file);

    return fs_file;
}

int write_file(char *file, char *name) {
    FILE *fs_file;

    fs_file = fopen(name, "wb");

    if(!fs_file) {
        return FILE_ERROR;
    }

    fwrite(file, sizeof(char), strlen(file), fs_file);

    fclose(fs_file);

    return NO_ERROR;
}

int write_file_b(char *file, char *name) {
    FILE *fs_file;

    fs_file = fopen(name, "wb");

    if(!fs_file) {
        return FILE_ERROR;
    }

    for (int i = 0; i < sb->disk_size; i++) {
        fputc(file[i], fs_file);
    }

    fputc(EOF, fs_file);

    fclose(fs_file);

    return NO_ERROR;
}

int load_fs(char *fs_name) {
    char *fs_file = read_file(fs_name);
    set_variables(fs_file);
    root_inode = 1;
    current_inode = 1;
    return NO_ERROR;
}

int init_fs(char *fs_name) {
    FILE *file = NULL;

    fs_file_name = fs_name;

    file = fopen(fs_name, "r");

    if (!file) {
        return FS_CREATED;
    }

    fclose(file);

    return load_fs(fs_name);
}

int32_t get_inodes_byte_count(int32_t size) {
    return ((size / INODES_RATIO) / sizeof(inode)) * sizeof(inode);
}

/* counts how many clusters are available */
int32_t get_data_blocks_count(int32_t size, int32_t inodes_byte_count) {
    return (((size - sizeof(superblock) - inodes_byte_count) / DATA_BLOCK_SIZE) / 8) * 8;
}

/* counts how many bytes will clusters use */
int32_t get_data_blocks_byte_count(int32_t data_blocks_count) {
    return data_blocks_count * DATA_BLOCK_SIZE;
}

/* counts how many bytes will the bitmap use */
int32_t get_bitmap_bytes_count(int32_t data_blocks_count) {
    return data_blocks_count / 8;
}

/* counts the final size of file system in bytes */
int32_t get_final_fs_bytes_count(int32_t bitmap_bytes, int32_t inodes_bytes, int32_t blocks_bytes) {
    return sizeof(superblock) + bitmap_bytes + inodes_bytes + blocks_bytes;
}

void init_inodes() {
    int i;
    for (i = 1; i < inodes_count; i++) {
        inodes[i].nodeid = FREE;
        inodes[i].file_size = 0;
        inodes[i].references = 0;
        inodes[i].isDirectory = false;
        inodes[i].direct1 = EMPTY;
        inodes[i].direct2 = EMPTY;
        inodes[i].direct3 = EMPTY;
        inodes[i].direct4 = EMPTY;
        inodes[i].direct5 = EMPTY;
        inodes[i].indirect1 = EMPTY;
        inodes[i].indirect2 = EMPTY;
    }
}

/* initialize root inode */
void init_root() {
    inode *inode = fs + sb->inode_start_address;
    inode->isDirectory = true;
    inode->nodeid = 1;
    inode->references = 1;
    inode->file_size = 0;
    inode->direct1 = 0;
    inode->direct2 = EMPTY;
    inode->direct3 = EMPTY;
    inode->direct4 = EMPTY;
    inode->direct5 = EMPTY;
    inode->indirect1 = EMPTY;
    inode->indirect2 = EMPTY;

    bitmap[0] = bitmap[0] | 1 << STARTING_BIT;

    directory_item *di1 = data_blocks;
    di1->inode = inode->nodeid;
    strcpy(di1->item_name, ".\0");

    directory_item *di2 = data_blocks + sizeof(directory_item);
    di2->inode = inode->nodeid;
    strcpy(di2->item_name, "..\0");

    root_inode = 1;
}

//UTILS

char *block_by_id(int block_index) {
    char * result = data_blocks + (block_index * sb->cluster_size);
    return result;
}

int get_free_inode() {
    int i;
    for (i = 0; i < inodes_count; i++) {
        if (inodes[i].nodeid == FREE) {
            inodes[i].nodeid = i + 1;
            return i + 1;
        }
    }
    return NOT_FOUND;
}

int get_free_block() {
    int i, j;

    for (i = 0; i < bitmap_size; i++) {
        for (j = 0; j < BITS_PER_BYTE; j++) {
            if ((bitmap[i] & 1 << (STARTING_BIT - j)) == 0) {
                bitmap[i] += 1 << (STARTING_BIT - j);
                return i * BITS_PER_BYTE + j;
            }
        }
    }
    return NOT_FOUND;
}

directory_item *get_free_dir_item(char *data_block) {
    directory_item *items = data_block;
    int i;
    for (i = 0; i < items_per_block; i++) {
        if (items[i].inode == FREE) {
            return &items[i];
        }
    }
}

char * format_path(char *path) {
    int len = strlen(path);
    char *name = calloc(sizeof(char), NAME_LEN), *name_start = NULL;
    int i, index = 0;

    for (i = len - 1; i >= 0; i--) {
        if (path[i] == '/') {
            index = i;
            break;
        }
    }

    if(index == 0 && path[0] == '/') {
        index += 1;
        name_start = path + index;
    }
    else if(path[index] == '/' && index != 0) {
        name_start = path + index + 1;
    }
    else {
        name_start = path;
    }

    if(strlen(name_start) > NAME_LEN-1) {
        strncpy(name, name_start, NAME_LEN-1);
    }
    else {
        strcpy(name, name_start);
    }

    for (i = index; i < len; i++) {
        path[i] = '\0';
    }

    return name;
}

int get_inode_block(inode *node, int index) {
    switch (index) {
        case 1: {
            return node->direct1;
        }
        case 2: {
            return node->direct2;
        }
        case 3: {
            return node->direct3;
        }
        case 4: {
            return node->direct4;
        }
        case 5: {
            return node->direct5;
        }
        default:
            break;
    }
    if (index > 5 && index < 134) {

        if (node->indirect1 == NOT_FOUND) {
            return NOT_FOUND;
        }

        int32_t *blocks = block_by_id(node->indirect1);
        return blocks[index];
    } else if (index > 133 && index < 262) {

        if (node->indirect2 == NOT_FOUND) {
            return NOT_FOUND;
        }

        int32_t *blocks = block_by_id(node->indirect2);
        return blocks[index];
    } else {
        return NOT_FOUND;
    }
}

int add_block_to_inode(inode *node, int block_id) {
    int i;
    int32_t *blocks;

    if (node->direct1 == EMPTY_BLOCK) {
        node->direct1 = block_id;
        return NO_ERROR;
    }
    if (node->direct2 == EMPTY_BLOCK) {
        node->direct2 = block_id;
        return NO_ERROR;
    }
    if (node->direct3 == EMPTY_BLOCK) {
        node->direct3 = block_id;
        return NO_ERROR;
    }
    if (node->direct4 == EMPTY_BLOCK) {
        node->direct4 = block_id;
        return NO_ERROR;
    }
    if (node->direct5 == EMPTY_BLOCK) {
        node->direct5 = block_id;
        return NO_ERROR;
    }

    if(node->indirect1 == EMPTY_BLOCK) {
        node->indirect1 = get_free_block();
    }
    blocks = block_by_id(node->indirect1);
    for (i = 0; i < block_indexes_per_block; i++) {
        if (blocks[i] == 0) {
            blocks[i] = block_id;
            return NO_ERROR;
        }
    }

    if(node->indirect2 == EMPTY_BLOCK) {
        node->indirect2 = get_free_block();
    }
    blocks = block_by_id(node->indirect2);
    for (i = 0; i < block_indexes_per_block; i++) {
        if (blocks[i] == 0) {
            blocks[i] = block_id;
            return NO_ERROR;
        }
    }

    return MEMORY_ERR;
}

void copy_block(char *block_source, char *block_dest) {
    if(!block_source || !block_dest) {
        return;
    }
    else {
        memcpy(block_dest, block_source, DATA_BLOCK_SIZE);
    }
}

directory_item *item_by_name(char *data_block, char *name) {
    directory_item *items = data_block;
    int i;
    for (i = 0; i < items_per_block; i++) {
        if (!strcmp(name, items[i].item_name)) {
            return &items[i];
        }
    }

    return NULL;
}

inode *inode_by_id(int inode_id) {
    int i;
    inode node;
    for (i = 0; i < inodes_count; i++) {
        node = inodes[i];
        if (inodes[i].nodeid == inode_id) {
            return &inodes[i];
        }
    }

    return NULL;
}

int inode_by_path(char *path) {
    int i, j, my_inode = NOT_FOUND, block, c = 0;
    char *tmp_path = NULL, name_buffer[50];
    directory_item *item;
    inode *node;

    memset(name_buffer, '\0', 50);

    if (strlen(path) == 0) {
        return current_inode;
    }

    if (strlen(path) > 0) {
        if (path[0] == '/') {
            my_inode = root_inode;
            tmp_path = path + 1;
        } else {
            my_inode = current_inode;
            tmp_path = path;
        }
    }

    node = inode_by_id(my_inode);

    for (i = 0; i < strlen(tmp_path); i++) {
        if (tmp_path[i] != '/') {
            name_buffer[c] = tmp_path[i];
            c++;
        }
        if(tmp_path[i] == '/' || i == strlen(tmp_path) - 1) {
            for (j = 1; j < MAX_BLOCKS_PER_INODE + 1; j++) {
                block = get_inode_block(node, j);
                if (block != EMPTY_BLOCK) {
                    item = item_by_name(block_by_id(block), name_buffer);
                    if (item && item->inode > 0) {
                        my_inode = item->inode;
                        node = inode_by_id(my_inode);
                        break;
                    } else {
                        return NOT_FOUND;
                    }
                }
            }
            c = 0;
            memset(name_buffer, '\0', 50);
        }
    }
    return my_inode;
}

int inode_parent(inode *node) {
    directory_item *item;

    if(node->isDirectory == true) {
        item = item_by_name(block_by_id(node->direct1), "..");

        return item->inode;
    }

    return NOT_FOUND;
}

void free_block(int block_index) {
    int i, j;
    char *block;

    if(block_index == -1) {
        return;
    }

    block = block_by_id(block_index);
    memset(block, '\0', DATA_BLOCK_SIZE);

    j = block_index % BITS_PER_BYTE;
    i = block_index / BITS_PER_BYTE;

    bitmap[i] -= 1 << (STARTING_BIT - j);
}

void free_inode(int inode_id) {
    inode *inode = NULL;

    if(inode_id == -1) {
        return;
    }

    inode = inode_by_id(inode_id);

    inode->nodeid = FREE;
    inode->file_size = 0;
    inode->references = 0;
    inode->isDirectory = false;
    free_block(inode->direct1);
    inode->direct1 = EMPTY_BLOCK;
    free_block(inode->direct2);
    inode->direct2 = EMPTY_BLOCK;
    free_block(inode->direct3);
    inode->direct3 = EMPTY_BLOCK;
    free_block(inode->direct4);
    inode->direct4 = EMPTY_BLOCK;
    free_block(inode->direct5);
    inode->direct5 = EMPTY_BLOCK;
    free_block(inode->indirect1);
    inode->indirect1 = EMPTY_BLOCK;
    free_block(inode->indirect2);
    inode->indirect2 = EMPTY_BLOCK;
}

//COMMANDS

int format_fs(int32_t size) {
    char *file = NULL;
    int32_t inodes_bytes, data_blocks_count, data_blocks_bytes, bitmap_bytes, final_fs_bytes,
            bitmap_start, inodes_start, data_blocks_start;

    inodes_bytes = get_inodes_byte_count(size);
    data_blocks_count = get_data_blocks_count(size, inodes_bytes);
    data_blocks_bytes = get_data_blocks_byte_count(data_blocks_count);
    bitmap_bytes = get_bitmap_bytes_count(data_blocks_count);

    bitmap_start = sizeof(superblock);
    inodes_start = bitmap_start + bitmap_bytes;
    data_blocks_start = inodes_start + inodes_bytes;

    final_fs_bytes = get_final_fs_bytes_count(bitmap_bytes, inodes_bytes, data_blocks_bytes);

    file = calloc(final_fs_bytes, sizeof(char));

    if (!file) {
        return FS_FORMAT_ERROR;
    }

    sb = file;
    sb->bitmap_start_address = bitmap_start;
    sb->inode_start_address = inodes_start;
    sb->data_start_address = data_blocks_start;
    sb->cluster_size = DATA_BLOCK_SIZE;
    sb->cluster_count = data_blocks_count;
    sb->disk_size = final_fs_bytes;
    strcpy(sb->signature, "mocjaks\0");
    strcpy(sb->volume_descriptor, "Mocjaks Inode FS\0");

    set_variables(file);
    init_inodes();
    init_root();

    return write_file_b(file, fs_file_name);
}

int create_dir(char *dir_name){
    char *name, *block;
    int i, new_id, block_id;
    inode *my_inode, *new_inode;
    directory_item *item;

    name = format_path(dir_name);
    int inode_id = inode_by_path(dir_name);

    if(inode_id == -1) {
        free(name);
        return NOT_FOUND;
    }

    my_inode = inode_by_id(inode_id);

    for (i = 1; i < MAX_BLOCKS_PER_INODE + 1; i++) {
        block_id = get_inode_block(my_inode, i);
        if(block_id != EMPTY_BLOCK) {
            item = item_by_name(block_by_id(block_id), name);
            if(item) {
                free(name);
                return EXISTS;
            }
        }
    }

    new_id = get_free_inode();

    if(new_id == -1) {
        free(name);
        return MEMORY_ERR;
    }

    new_inode = inode_by_id(new_id);
    new_inode->references = 1;
    new_inode->isDirectory = true;
    new_inode->file_size = 0;

    block_id = get_free_block();
    block = block_by_id(block_id);

    directory_item *di1 = block;
    di1->inode = new_inode->nodeid;
    strcpy(di1->item_name, ".\0");

    directory_item *di2 = block + sizeof(directory_item);
    di2->inode = my_inode->nodeid;
    strcpy(di2->item_name, "..\0");

    new_inode->direct1 = block_id;

    for (i = 1; i < MAX_BLOCKS_PER_INODE + 1; i++) {
        block_id = get_inode_block(my_inode, i);
        if(block_id != EMPTY_BLOCK) {
            item = get_free_dir_item(block_by_id(block_id));
            if(item) {
                strcpy(item->item_name, name);
                item->inode = new_inode->nodeid;
                free(name);
                return NO_ERROR;
            }
        }
    }
    free(name);
    return NOT_FOUND;
}

int remove_dir(char *path){
    int inode_id, i, j, block;
    inode *source_node, *parent_node;
    directory_item *items, *item;
    char *name;

    inode_id = inode_by_path(path);

    if(inode_id == -1) {
        return NOT_FOUND;
    }

    source_node = inode_by_id(inode_id);

    if(source_node->isDirectory == false) {
        return NOT_FOUND;
    }

    for (i = 1; i < MAX_BLOCKS_PER_INODE + 1; i++) {
        block = get_inode_block(source_node, i);
        if(block != EMPTY_BLOCK) {
            items = block_by_id(block);
            for (j = 2; j < items_per_block; j++) {
                if(items[j].inode != 0) {
                    return DIR_NOT_EMPTY;
                }
            }
        }
    }

    name = format_path(path);

    int blck = get_inode_block(source_node, 1);
    parent_node = inode_by_id(blck);
    for (i = 1; i < MAX_BLOCKS_PER_INODE + 1; i++) {
        block = get_inode_block(parent_node, i);
        if(block != EMPTY_BLOCK) {
            item = item_by_name(block_by_id(block), name);
            if(item) {
                if(item->inode > 0) {
                    if(inode_by_id(item->inode)->isDirectory == true) {
                        memset(item->item_name, '\0', 12);
                        free_inode(item->inode);
                        item->inode = 0;
                        free(name);
                        return NO_ERROR;
                    }
                }
            }
        }
    }
    return NO_ERROR;
}

int change_dir(char *path) {
    int inode_id;
    char *name;

    if(!strcmp(path, "..")) {
        inode_id = inode_parent(inode_by_id(current_inode));
        name = format_path(current_path);
        free(name);
    }
    else {
        inode_id = inode_by_path(path);

        if(inode_id == -1) {
            return NOT_FOUND;
        }

        if(inode_by_id(inode_id)->isDirectory == false) {
            return NOT_FOUND;
        }

        if(current_inode != root_inode) {
            strcat(current_path, "/");
        }

        if(path[0] == '/') {
            memset(current_path, '\0', BUFFER_SIZE);
        }

        strcat(current_path, path);
    }

    current_inode = inode_id;
    return NO_ERROR;
}

int print_dir(char *path) {
    int inode_id, i, j, block;
    inode *source_node;
    directory_item *items;
    char * name;

    if (path) {
        inode_id = inode_by_path(path);

        if (inode_id == -1) {
            return NOT_FOUND;
        }

        if (inode_by_id(inode_id)->isDirectory == false) {
            return NOT_FOUND;
        }

        source_node = inode_by_id(inode_id);
    }
    else {
        source_node = inode_by_id(current_inode);
    }

    if(source_node->isDirectory == false) {
        return NOT_FOUND;
    }

    for (i = 1; i < MAX_BLOCKS_PER_INODE + 1; i++) {
        block = get_inode_block(source_node, i);
        if(block != EMPTY_BLOCK) {
            items = block_by_id(block);
            for (j = 0; j < items_per_block; j++) {
                if(items[j].inode != 0) {
                    name = items[j].item_name;
                    if(inode_by_id(items[j].inode)->isDirectory == true) {
                        printf("DIR  %s\n", name);
                    }
                    else {
                        printf("FILE %s\n", name);
                    }
                }
            }
        }
        else {
            break;
        }
    }

    return NO_ERROR;
}

int copy_file(char *path, char *dest) {
    int source_node_id, dest_node_id, i, block, new_node_id;
    inode *dest_node, *new_inode, *source_node;
    char *name;
    directory_item *item;

    new_node_id = get_free_inode();

    new_inode = inode_by_id(new_node_id);

    source_node_id = inode_by_path(path);

    if(source_node_id == -1) {
        return NOT_EXIST;
    }

    source_node = inode_by_id(source_node_id);

    if(source_node->isDirectory == true) {
        return NOT_EXIST;
    }

    new_inode->file_size = source_node->file_size;
    new_inode->references = 1;
    new_inode->isDirectory = false;

    if(source_node->direct1 != EMPTY_BLOCK) {
        new_inode->direct1 = get_free_block();
        copy_block(block_by_id(source_node->direct1), block_by_id(new_inode->direct1));
    }
    if(source_node->direct2 != EMPTY_BLOCK) {
        new_inode->direct2 = get_free_block();
        copy_block(block_by_id(source_node->direct2), block_by_id(new_inode->direct2));
    }
    if(source_node->direct3 != EMPTY_BLOCK) {
        new_inode->direct3 = get_free_block();
        copy_block(block_by_id(source_node->direct3), block_by_id(new_inode->direct3));
    }
    if(source_node->direct4 != EMPTY_BLOCK) {
        new_inode->direct4 = get_free_block();
        copy_block(block_by_id(source_node->direct4), block_by_id(new_inode->direct4));
    }
    if(source_node->direct5 != EMPTY_BLOCK) {
        new_inode->direct5 = get_free_block();
        copy_block(block_by_id(source_node->direct5), block_by_id(new_inode->direct5));
    }
    if(source_node->indirect1 != EMPTY_BLOCK) {
        new_inode->indirect1 = get_free_block();
        copy_block(block_by_id(source_node->indirect1), block_by_id(new_inode->indirect1));
    }
    if(source_node->indirect2 != EMPTY_BLOCK) {
        new_inode->indirect2 = get_free_block();
        copy_block(block_by_id(source_node->indirect2), block_by_id(new_inode->indirect2));
    }

    name = format_path(dest);

    dest_node_id = inode_by_path(dest);

    if(dest_node_id == -1) {
        free(name);
        return NOT_FOUND;
    }

    dest_node = inode_by_id(dest_node_id);

    for (i = 1; i < MAX_BLOCKS_PER_INODE + 1; i++) {
        block = get_inode_block(dest_node, i);
        if(block != EMPTY_BLOCK) {
            item = get_free_dir_item(block_by_id(block));
            if(item) {
                strcpy(item->item_name, name);
                item->inode = new_node_id;
                free(name);
                return NO_ERROR;
            }
        }
    }
    free(name);
    return NOT_FOUND;
}

int move_file(char *path, char *dest) {
    int source_node_id, dest_node_id, i, block;
    inode *source_node, *dest_node, *parent_node;
    char *source_name = NULL, *dest_name = NULL;
    directory_item *item;

    source_node_id = inode_by_path(path);

    if(source_node_id == -1) {
        return NOT_EXIST;
    }

    source_node = inode_by_id(source_node_id);

    if(source_node->isDirectory == true) {
        return NOT_EXIST;
    }

    source_name = format_path(path);

    parent_node = inode_by_id(get_inode_block(source_node, 1));
    for (i = 1; i < MAX_BLOCKS_PER_INODE + 1; i++) {
        block = get_inode_block(parent_node, i);
        if(block != EMPTY_BLOCK) {
            item = item_by_name(block_by_id(block), source_name);
            if(item) {
                memset(item->item_name, '\0', 12);
                item->inode = 0;
                break;
            }
        }
    }

    dest_name = format_path(dest);

    dest_node_id = inode_by_path(dest);

    if(dest_node_id == -1) {
        free(source_name);
        free(dest_name);
        return NOT_FOUND;
    }

    dest_node = inode_by_id(dest_node_id);

    for (i = 1; i < MAX_BLOCKS_PER_INODE + 1; i++) {
        block = get_inode_block(dest_node, i);
        if(block != EMPTY_BLOCK) {
            item = get_free_dir_item(block_by_id(block));
            if(item) {
                strcpy(item->item_name, dest_name);
                item->inode = source_node_id;
                free(source_name);
                free(dest_name);
                return NO_ERROR;
            }
        }
    }
    free(source_name);
    free(dest_name);
    return NOT_FOUND;
}

int remove_file(char *path) {
    char *source_name;
    int source_node_id, block, i, source_file_id;
    inode *source_node;
    directory_item *item;

    source_file_id = inode_by_path(path);

    source_name = format_path(path);

    source_node_id = inode_by_path(path);

    if(source_node_id == -1 || source_file_id == -1) {
        free(source_name);
        return NOT_EXIST;
    }

    if(inode_by_id(source_file_id)->isDirectory == true) {
        free(source_name);
        return NOT_EXIST;
    }

    source_node = inode_by_id(source_node_id);

    for (i = 1; i < MAX_BLOCKS_PER_INODE + 1; i++) {
        block = get_inode_block(source_node, i);
        if(block != EMPTY_BLOCK) {
            item = item_by_name(block_by_id(block), source_name);
            if(item) {
                if(item->inode > 0) {
                    if (inode_by_id(item->inode)->isDirectory == false) {
                        memset(item->item_name, '\0', 12);
                        free_inode(item->inode);
                        item->inode = 0;
                        free(source_name);
                        return NO_ERROR;
                    }
                }
            }
        }
    }
    free(source_name);
    return NOT_FOUND;
}

int in_copy_file(char *file_name, char *path) {
    char *file = read_file(file_name), *file_pointer = file, *name;
    int temp_size, i, file_blocks[MAX_BLOCKS_PER_INODE], target_inode_id;
    int number_of_blocks, new_inode_id;
    inode *new_inode, *target_inode;
    directory_item *item;

    if(!file) {
        return NOT_EXIST;
    }

    name = format_path(path);

    target_inode_id = inode_by_path(path);

    if(target_inode_id == -1) {
        free(file);
        free(name);
        return NOT_FOUND;
    }

    target_inode = inode_by_id(target_inode_id);

    if(target_inode->isDirectory == false) {
        free(file);
        free(name);
        return NOT_FOUND;
    }

    number_of_blocks = strlen(file) / DATA_BLOCK_SIZE;

    if(number_of_blocks > 261) {
        free(file);
        free(name);
        return MEMORY_ERR;
    }

    if(strlen(file) > number_of_blocks * DATA_BLOCK_SIZE) {
        number_of_blocks += 1;
    }

    temp_size = strlen(file);

    for (i = 0; i < number_of_blocks; i++) {
        file_blocks[i] = get_free_block();
        if(temp_size > DATA_BLOCK_SIZE) {
            memcpy(block_by_id(file_blocks[i]), file_pointer, DATA_BLOCK_SIZE);
            file_pointer += DATA_BLOCK_SIZE;
            temp_size -= DATA_BLOCK_SIZE;
        }
        else {
            strncpy(block_by_id(file_blocks[i]), file_pointer, temp_size);
            file_pointer += temp_size;
        }
    }

    free(file);

    new_inode_id = get_free_inode();

    if(new_inode_id == NOT_FOUND) {
        free(name);
        return MEMORY_ERR;
    }

    new_inode = inode_by_id(new_inode_id);
    new_inode->file_size = strlen(file);
    new_inode->isDirectory = false;
    new_inode->references = 1;

    for (i = 0; i < number_of_blocks; i++) {
        if(add_block_to_inode(new_inode, file_blocks[i])) {
            free(name);
            return MEMORY_ERR;
        }
    }

    for (i = 1; i < MAX_BLOCKS_PER_INODE + 1; i++) {
        if(get_inode_block(target_inode, i) != EMPTY_BLOCK) {
            item = get_free_dir_item(block_by_id(get_inode_block(target_inode, i)));
            if(item) {
                item->inode = new_inode->nodeid;
                strcpy(item->item_name, name);
                break;
            }
        }
    }

    free(name);
    return NO_ERROR;
}

int out_copy_file(char *file_name, char *path){
    int i, block, my_inode, j;
    inode *node;
    char *text;
    char *string;
    char *buffer;

    my_inode = inode_by_path(file_name);

    if(my_inode == -1) {
        return NOT_FOUND;
    }

    if(inode_by_id(my_inode)->isDirectory == true) {
        return NOT_FOUND;
    }

    node = inode_by_id(my_inode);

    text = calloc(node->file_size + 1, sizeof(char));

    for (i = 1; i < MAX_BLOCKS_PER_INODE + 1; i++) {
        block = get_inode_block(node, i);
        string = block_by_id(block);
        buffer = calloc(DATA_BLOCK_SIZE, sizeof(char ));
        for(j = 0; j < DATA_BLOCK_SIZE; j++){
            buffer[j] = string[j];
        }
        printf("%s", buffer);
        strcat(text, buffer);
        free(buffer);
    }

    printf("%s", text);

    write_file(text, path);
    printf("\n");
    return NO_ERROR;
}

int print_file(char *path) {
    int i, j, my_inode, block;
    char* string;
    inode *node;

    my_inode = inode_by_path(path);

    if(my_inode == NOT_FOUND) {
        return NOT_FOUND;
    }

    if(inode_by_id(my_inode)->isDirectory == true) {
        return NOT_FOUND;
    }

    node = inode_by_id(my_inode);

    for (i = 1; i < MAX_BLOCKS_PER_INODE + 1; i++) {
        block = get_inode_block(node, i);
        if(block != EMPTY_BLOCK) {
            string = block_by_id(block);
            for (j = 0; j < DATA_BLOCK_SIZE; j++){
                printf("%c", string[j]);
            }
        }
        else {
            break;
        }
    }

    printf("\n");
    return NO_ERROR;
}
