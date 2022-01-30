/*
 * Created by Tomas Linhart on 4. 2. 2021.
 */

#include "vfs.h"
#include "utils.h"
#include "constants.h"
#include "directory.h"
#include "macros.h"
#include <stdint.h>

/*
 * Loops through all data blocks, storing addressess of each free data block. Returns array of addressess or NULL when not enough blocks found.
 */
int32_t *find_free_data_blocks(VFS** vfs, int count) {
    int i, found_blocks = 0;
    int32_t *blocks = calloc(count, sizeof(int32_t));

    e_return_value_on_null(blocks, MEMORY_ERROR_MSG, NULL);

    /* Find all data blocks */
    for (i = 1; i < (*vfs)->superblock->data_cluster_count; i++) { /* Skip root */
        if ((*vfs)->data_bitmap[i] == 0) {
            blocks[found_blocks] = i;
            found_blocks++;
            if (found_blocks == count) {
                return blocks;
            }
        }
    }

    free(blocks);
    return NULL;
}

/*
 * Loop through inodes, returning ID of first Free node
 */
int32_t find_free_inode(VFS** vfs) {
    int i;

    for (i = 1; i < (*vfs)->superblock->inode_count; i++) {
        if ((*vfs)->inodes[i].nodeid == ID_ITEM_FREE) {
            return i;
        }
    }
    return ERROR_CODE;
}

/*
 * Returns array of data blocks for inode with specified ID, saves block count and length of data stored in last block at given addresses
 */
int32_t *get_data_blocks(VFS** vfs, int32_t nodeid, int *block_count, int *rest) {
    int32_t *data_blocks;
    int32_t number;
    int i, tmp, counter;

    int max_numbers = 2 * (CLUSTER_SIZE / 4) + 5;	/*  Maximum data blocks */
    inode *node = &((*vfs)->inodes[nodeid]);

    if (node->isDirectory) {	/*  If item is directory we cannot determine size. We have to loop through all blocks until we reach end */
        counter = 0;
        data_blocks = calloc(max_numbers, sizeof(int32_t));

        if (node->direct1 != ID_ITEM_FREE) data_blocks[counter++] = node->direct1;
        if (node->direct2 != ID_ITEM_FREE) data_blocks[counter++] = node->direct2;
        if (node->direct3 != ID_ITEM_FREE) data_blocks[counter++] = node->direct3;
        if (node->direct4 != ID_ITEM_FREE) data_blocks[counter++] = node->direct4;
        if (node->direct5 != ID_ITEM_FREE) data_blocks[counter++] = node->direct5;

        if (node->indirect1 != ID_ITEM_FREE) {
            seek_data_cluster(vfs, node->indirect1);
            for (i = 0; i < CLUSTER_SIZE / 4; i++) {
                int32_read_vfs(vfs, &number);
                if (number > 0) {
                    data_blocks[counter++] = number;
                } else {
                    break;
                }
            }
        }

        if (node->indirect2 != ID_ITEM_FREE) {
            seek_data_cluster(vfs, node->indirect2);
            for (i = 0; i < CLUSTER_SIZE / 4; i++) {
                int32_read_vfs(vfs, &number);
                if (number > 0) {
                    data_blocks[counter++] = number;
                }
            }
        }

        *block_count = counter;
    } else {	/*  If item is file */
        *block_count = node->file_size / CLUSTER_SIZE;

        if (rest != NULL) {
            *rest = node->file_size % CLUSTER_SIZE;
        }

        if (node->file_size % CLUSTER_SIZE != 0) {      /* Need one more block to save rest of the file */
            (*block_count)++;
        }

        data_blocks = calloc((*block_count), sizeof(int32_t));

        data_blocks[0] = node->direct1;
        if (*block_count > 1) data_blocks[1] = node->direct2;
        if (*block_count > 2) data_blocks[2] = node->direct3;
        if (*block_count > 3) data_blocks[3] = node->direct4;
        if (*block_count > 4) data_blocks[4] = node->direct5;

        if (*block_count > 5) {
            if (*block_count > (CLUSTER_SIZE / 4) + 5) {	                                                            /*  Uses second indirect reference */
                seek_data_cluster(vfs, node->indirect1);                                                                /*  Move to first indirect reference */
                read_vfs(vfs, &data_blocks[5], sizeof(int32_t), CLUSTER_SIZE / sizeof(int32_t));            /*  Read all INT32 address from cluster */

                tmp = *block_count - (CLUSTER_SIZE / 4) - 5;                                                            /*  Number of blocks to read */
                seek_data_cluster(vfs, node->indirect2);                                                                /*  Move to second indirect reference */
                read_vfs(vfs, &data_blocks[(CLUSTER_SIZE / 4) + 5], sizeof(int32_t), tmp);                         /*  Read rest of indirect references from cluster */
            }
            else {	/*  Only first indirect reference is used */
                tmp = *block_count - 5;
                seek_data_cluster(vfs, node->indirect1);
                read_vfs(vfs, &data_blocks[5], sizeof(int32_t), tmp);
            }
        }
    }

    return data_blocks;
}

/*
 * Updates bitmap in VFS and saves the bitmap in VFS file.
 */
void update_bitmap_in_file(VFS** vfs, dir_item *item, int8_t value, int32_t *data_blocks, int b_count) {
    int i, block_count;
    int32_t *blocks;

    if (!data_blocks) {
        blocks = get_data_blocks(vfs, item->inode, &block_count, NULL);
    }
    else {
        blocks = data_blocks;
        block_count = b_count;
    }

    /* Write all blocks */
    for (i = 0; i < block_count; i++) {
        (*vfs)->data_bitmap[blocks[i]] = value;
        seek_set(vfs, (*vfs)->superblock->bitmap_start_address + blocks[i]);
        int8_write_vfs(vfs, &value);
    }


    /* Indirect 1 data block */
    if ((*vfs)->inodes[item->inode].indirect1 != ID_ITEM_FREE) {
        (*vfs)->data_bitmap[(*vfs)->inodes[item->inode].indirect1] = value;
        seek_set(vfs, (*vfs)->superblock->bitmap_start_address + (*vfs)->inodes[item->inode].indirect1);
        int8_write_vfs(vfs, &value);
    }
    /* Indirect 2 data block */
    if ((*vfs)->inodes[item->inode].indirect2 != ID_ITEM_FREE) {
        (*vfs)->data_bitmap[(*vfs)->inodes[item->inode].indirect2] = value;
        seek_set(vfs, (*vfs)->superblock->bitmap_start_address + (*vfs)->inodes[item->inode].indirect2);
        int8_write_vfs(vfs, &value);
    }

    flush_vfs(vfs);
}

void load_directory_from_vfs(VFS** vfs, directory *dir, int id) {
    int i, j, block_count;
    int inode_count = 64;	/* Maximum count of i-nodes in one data block */
    int32_t *data_blocks;
    int32_t node_id;
    char filename[12];
    directory *new_directory;
    dir_item *item, *temp_subdir;
    dir_item **last_subdir_address = &(dir->subdir);
    dir_item **last_file_address = &(dir->file);

    /* Get data blocks of this directory */
    data_blocks = get_data_blocks(vfs, dir->current->inode, &block_count, NULL);

    for (i = 0; i < block_count; i++) {
        seek_data_cluster(vfs, data_blocks[i]);
        for (j = 0; j < inode_count; j++) {
            int32_read_vfs(vfs, &node_id);
            if (node_id > 0) { /* Invalid inode, or root */
                read_vfs(vfs, filename, sizeof(filename), 1);
                item = new_directory_item(node_id, filename);
                if ((*vfs)->inodes[node_id].isDirectory) {
                    *last_subdir_address = item;
                    last_subdir_address = &(item->next);
                } else {
                    *last_file_address = item;
                    last_file_address = &(item->next);
                }
            } else { /* Skip */
                seek_cur(vfs, sizeof(filename));
            }
        }
    }
    free(data_blocks);

    /* Recursively load subdirs */
    temp_subdir = dir->subdir;
    while (temp_subdir != NULL) {
        new_directory = calloc(1, sizeof(directory));
        new_directory->parent = dir;
        new_directory->current = temp_subdir;
        new_directory->subdir = NULL;
        new_directory->file = NULL;

        (*vfs)->all_dirs[temp_subdir->inode] = new_directory;
        load_directory_from_vfs(vfs, new_directory, temp_subdir->inode);

        temp_subdir = temp_subdir->next;
    }
}

/*
 * Load VFS from VFS file
 */
void load_vfs(VFS **vfs) {
    (*vfs)->superblock = calloc(1, sizeof(superblock));
    e_return_on_null((*vfs)->superblock, MEMORY_ERROR_MSG);

    /* Read superblock from file */
    rewind_vfs(vfs);
    read_vfs(vfs, &((*vfs)->superblock->signature), sizeof(char) * SIGNATURE_LENGTH, 1);
    int32_read_vfs(vfs, &((*vfs)->superblock->disk_size));
    int32_read_vfs(vfs, &((*vfs)->superblock->cluster_size));
    int32_read_vfs(vfs, &((*vfs)->superblock->cluster_count));
    int32_read_vfs(vfs, &((*vfs)->superblock->inode_count));
    int32_read_vfs(vfs, &((*vfs)->superblock->bitmap_cluster_count));
    int32_read_vfs(vfs, &((*vfs)->superblock->inode_cluster_count));
    int32_read_vfs(vfs, &((*vfs)->superblock->data_cluster_count));
    int32_read_vfs(vfs, &((*vfs)->superblock->bitmap_start_address));
    int32_read_vfs(vfs, &((*vfs)->superblock->inode_start_address));
    int32_read_vfs(vfs, &((*vfs)->superblock->data_start_address));

    /* Prepare bitmap, i-nodes and pointers to directories */
    (*vfs)->data_bitmap = calloc(1, (*vfs)->superblock->cluster_count);
    e_return_on_null((*vfs)->data_bitmap, MEMORY_ERROR_MSG);

    (*vfs)->inodes = calloc((*vfs)->superblock->inode_count, sizeof(inode));
    e_return_on_null((*vfs)->inodes, MEMORY_ERROR_MSG);

    (*vfs)->all_dirs = calloc((*vfs)->superblock->inode_count, sizeof(directory *)); /* Kdyby všechny položky ve FS byly složk */
    e_return_on_null((*vfs)->all_dirs, MEMORY_ERROR_MSG);

    /* Load bitmap from file */
    seek_set(vfs, (*vfs)->superblock->bitmap_start_address);
    read_vfs(vfs, (*vfs)->data_bitmap, sizeof(int8_t), (*vfs)->superblock->data_cluster_count);

    /* Load all I-Nodes */
    seek_set(vfs, (*vfs)->superblock->inode_start_address);
    for (int i = 0; i < (*vfs)->superblock->inode_count; i++) {
        seek_set(vfs, (*vfs)->superblock->inode_start_address + (i * INODE_SIZE));
        int32_read_vfs(vfs, &((*vfs)->inodes[i].nodeid));
        seek_set(vfs, (*vfs)->superblock->inode_start_address + (i * INODE_SIZE) + 4);
        int8_read_vfs(vfs, &((*vfs)->inodes[i].isDirectory));
        seek_set(vfs, (*vfs)->superblock->inode_start_address + (i * INODE_SIZE) + 5);
        int8_read_vfs(vfs, &((*vfs)->inodes[i].references));
        seek_set(vfs, (*vfs)->superblock->inode_start_address + (i * INODE_SIZE) + 6);
        int32_read_vfs(vfs, &((*vfs)->inodes[i].file_size));
        seek_set(vfs, (*vfs)->superblock->inode_start_address + (i * INODE_SIZE) + 10);
        int32_read_vfs(vfs, &((*vfs)->inodes[i].direct1));
        seek_set(vfs, (*vfs)->superblock->inode_start_address + (i * INODE_SIZE) + 14);
        int32_read_vfs(vfs, &((*vfs)->inodes[i].direct2));
        seek_set(vfs, (*vfs)->superblock->inode_start_address + (i * INODE_SIZE) + 18);
        int32_read_vfs(vfs, &((*vfs)->inodes[i].direct3));
        seek_set(vfs, (*vfs)->superblock->inode_start_address + (i * INODE_SIZE) + 22);
        int32_read_vfs(vfs, &((*vfs)->inodes[i].direct4));
        seek_set(vfs, (*vfs)->superblock->inode_start_address + (i * INODE_SIZE) + 26);
        int32_read_vfs(vfs, &((*vfs)->inodes[i].direct5));
        seek_set(vfs, (*vfs)->superblock->inode_start_address + (i * INODE_SIZE) + 30);
        int32_read_vfs(vfs, &((*vfs)->inodes[i].indirect1));
        seek_set(vfs, (*vfs)->superblock->inode_start_address + (i * INODE_SIZE) + 34);
        int32_read_vfs(vfs, &((*vfs)->inodes[i].indirect2));
    }


    /* Create root */
    directory *root = calloc(1, sizeof(directory));
    e_return_on_null(root, MEMORY_ERROR_MSG);

    root->current = new_directory_item(0, "/");
    root->parent = root;
    root->subdir = NULL;
    root->file = NULL;

    (*vfs)->current_dir = root;	/* Set root as working directory */
    (*vfs)->all_dirs[0] = root;

    /* Load directories */
    load_directory_from_vfs(vfs, root, 0);

    printf(VFS_LOAD_SUCCESS);
}

/*
 * Goes through path specified in string and moves in directories based on command. Returns struct of found directory, or NULL when not found
 */
directory *find_directory(VFS** vfs, char *path) {
    bool found;
    char *part;	/* Current part of the path */
    directory *dir;
    dir_item *item;

    if (path[0] == '/') {	        /* Absolute path */
        dir = (*vfs)->all_dirs[0];  /* Set current dir to root */
    } else {					    /* Relative path */
        dir = (*vfs)->current_dir;  /* Use current dir */
    }

    part = strtok(path, PATH_DELIMETER);
    while (part != NULL) {
        if (strcmp(part, ".") == 0) {	    /* The same dir */
            part = strtok(NULL, PATH_DELIMETER);
            continue;
        }
        else if (strcmp(part, "..") == 0) {	/* Go to the parent directory */
            dir = dir->parent;
            part = strtok(NULL, PATH_DELIMETER);
            continue;
        }
        else {
            found = false;
            item = dir->subdir;
            while (item != NULL) {
                if (streq(part, (*vfs)->all_dirs[item->inode]->current->item_name)) {
                    dir = (*vfs)->all_dirs[item->inode];
                    part = strtok(NULL, PATH_DELIMETER);
                    found = true;
                    break;
                }
                item = item->next;
            }

            if (found == false) {	/*  No such directory wasn't found */
                return NULL;
            }
        }
    }
    return dir;
}

/*
 * Initializes VFS struct when started up
 */
void initialize_vfs(VFS **vfs, char *vfs_name) {
    (*vfs) = calloc(1, sizeof(VFS));
    (*vfs)->name = vfs_name;

    if (!file_exists(vfs_name)) {
        printf(START_NEEDS_FORMAT_MSG);
        (*vfs)->is_formatted = false;
    } else {
        FILE *temp_vfs_file = fopen(vfs_name, "rb+");
        (*vfs)->is_formatted = true;
        (*vfs)->vfs_file = temp_vfs_file;

        load_vfs(vfs);
    }
}

bool vfs_exists(directory *dir, char *name) {
    dir_item *item;

    /* Loop through files of the directory */
    item = dir->file;
    while (item != NULL) {
        if (streq(name, item->item_name)) {
            return true;
        }
        item = item->next;
    }

    /* Loop through subdirs of the directory */
    item = dir->subdir;
    while (item != NULL) {
        if (streq(name, item->item_name)) {
            return true;
        }
        item = item->next;
    }
    return false;
}

/*
 * Removes directory from VFS file
 */
int remove_directory_from_file(VFS** vfs, directory *dir, dir_item *item) {
    int block_number, j, block_count, item_count, rest, found = 0;
    int32_t *blocks, number, count, zero = 0;
    int empty[4];
    int max_items_in_block = 64;
    int32_t nodeid;

    memset(empty, 0, sizeof(empty));

    /* Get data blocks */
    blocks = get_data_blocks(vfs, dir->current->inode, &block_count, &rest);

    for (block_number = 0; block_number < block_count; block_number++) {
        seek_data_cluster(vfs, blocks[block_number]);

        item_count = 0;	// Counter of items in this data block
        for (j = 0; j < max_items_in_block; j++) {
            int32_read_vfs(vfs, &nodeid);
            if (nodeid > 0) {
                item_count++;
            }

            if (!found) {
                if (nodeid == (item->inode)) {
                    seek_cur(vfs, -4);
                    write_vfs(vfs, &empty, sizeof(empty), 1);
                    flush_vfs(vfs);
                    found = 1;

                    if (item_count > 1) break;
                }
            }
        }

        if (found) {	/* Verify if data block is free - If yes remove reference to it */
            if (item_count == 1) {
                inode *node = &((*vfs)->inodes[item->inode]);

                if (node->direct1 != block_number) {	/* Don't remove first direct */
                    if (node->direct2 == block_number) {
                        node->direct2 = ID_ITEM_FREE;
                    }
                    else if (node->direct3 == block_number) {
                        node->direct3 = ID_ITEM_FREE;
                    }
                    else if (node->direct4 == block_number) {
                        node->direct4 = ID_ITEM_FREE;
                    }
                    else if (node->direct5 == block_number) {
                        node->direct5 = ID_ITEM_FREE;
                    }
                    else {
                        for (int i = 0; i < 2; i++) {
                            if (i == 0)	// Go through indirect1
                                seek_data_cluster(vfs, node->indirect1);
                            else 		// Go through indirect2
                                seek_data_cluster(vfs, node->indirect2);

                            count = 0;
                            found = 0;
                            for (j = 0; j < INT32_COUNT_IN_BLOCK; j++) {
                                int32_read_vfs(vfs, &number);
                                if (number > 0)
                                    count++;
                                if (!found) {
                                    if (number == block_number) {
                                        found = 1;
                                        seek_cur(vfs, NEGATIVE_SIZE_OF_INT32);
                                        int32_write_vfs(vfs, &zero);
                                        flush_vfs(vfs);
                                        if (count > 1)
                                            break;
                                    }
                                }
                            }

                            if (found) {
                                /* Remove indirect references if they are empty */
                                if (count == 1) {
                                    if (i == 0) {
                                        node->indirect1 = ID_ITEM_FREE;
                                    }
                                    else {
                                        node->indirect2 = ID_ITEM_FREE;
                                    }
                                }
                                break;
                            }
                        }
                    }

                    update_bitmap_in_file(vfs, item, 0, NULL, 0);
                    write_inode_to_vfs(vfs, item->inode);
                }
            }

            free(blocks);
            return NO_ERROR_CODE;
        }
    }

    return ERROR_CODE;
}

/*
 * Creates directory in VFS file
 */
int create_directory_in_file(VFS** vfs, directory *dir, dir_item *item) {
    int i, j, block_count;
    int32_t *blocks, *free_block;
    int max_items_in_block = 64;
    int32_t nodeid;
    inode *dir_node;

    /* Get data blocks */
    blocks = get_data_blocks(vfs, dir->current->inode, &block_count, NULL);

    for (i = 0; i < block_count; i++) {
        seek_data_cluster(vfs, blocks[i]);
        for (j = 0; j < max_items_in_block; j++) {
            int32_read_vfs(vfs, &nodeid);
            if (nodeid == 0) {
                seek_cur(vfs, -4); /* Rewind back for a size of int32_t (4 bytes) */
                int32_write_vfs(vfs, &(item->inode)); /* Store address of inode */
                write_vfs(vfs, item->item_name, sizeof(item->item_name), 1); /* Store name of folder */
                flush_vfs(vfs);
                free(blocks);
                return NO_ERROR_CODE;
            } else {
                seek_cur(vfs, FILENAME_LENGTH);	/* Skip filename */
            }
        }
    }

    /* No free space left, we need to assign new data cluster */
    free_block = find_free_data_blocks(vfs, 1);
    if (!free_block) {
        return ERROR_CODE;
    }


    dir_node = &((*vfs)->inodes[dir->current->inode]);

    if (dir_node->direct1 == ID_ITEM_FREE) dir_node->direct1 = free_block[0];
    else if (dir_node->direct2 == ID_ITEM_FREE) dir_node->direct2 = free_block[0];
    else if (dir_node->direct3 == ID_ITEM_FREE) dir_node->direct3 = free_block[0];
    else if (dir_node->direct4 == ID_ITEM_FREE) dir_node->direct4 = free_block[0];
    else if (dir_node->direct5 == ID_ITEM_FREE) dir_node->direct5 = free_block[0];
    else {
        free(free_block);
        free_block = find_free_data_blocks(vfs, 2);	/* Use indirect reference (need 2 free blocks - one to store addresses in indirect reference and one for the dirs) */
        if (free_block == NULL) return ERROR_CODE;

        if (dir_node->indirect1 == ID_ITEM_FREE) {
            dir_node->indirect1 = free_block[1];
        }
        else if (dir_node->indirect2 == ID_ITEM_FREE) {
            dir_node->indirect2 = free_block[1];
        }

        seek_data_cluster(vfs, free_block[1]);
        int32_write_vfs(vfs, &(free_block[0]));
    }

    seek_data_cluster(vfs, free_block[0]);
    int32_write_vfs(vfs, &(item->inode));
    write_vfs(vfs, item->item_name, sizeof(item->item_name), 1);

    flush_vfs(vfs);
    update_bitmap_in_file(vfs, dir->current, 1, NULL, 0);
    write_inode_to_vfs(vfs, dir->current->inode);
    free(free_block);
    free(blocks);
    return NO_ERROR_CODE;
}

/*
 * Updates directory in the VFS file - determines if it deletes or creates directory
 */
int update_directory_in_file(VFS** vfs, directory *dir, dir_item *item, bool create) {
    if (create == true) {	// Store item (find free space)
        return create_directory_in_file(vfs, dir, item);
    }
    else {	// Remove item (find the item with the specific id)
        return remove_directory_from_file(vfs, dir, item);
    }

    return ERROR_CODE;
}

/*
 * goes up the hierarchy updating all size sof the directories
 */
void update_sizes_in_file(VFS** vfs, directory *dir, int32_t size) {
    directory *d = dir;
    while (d != (*vfs)->all_dirs[0]) {
        (*vfs)->inodes[d->current->inode].file_size += size;
        write_inode_to_vfs(vfs, d->current->inode);
        d = d->parent;
    }

    /* Update root */
    (*vfs)->inodes[d->current->inode].file_size += size;
    write_inode_to_vfs(vfs, d->current->inode);
}

/*
 * Initializes new i-node. Returns index of the last block.
 */
int initialize_inode(VFS** vfs, int32_t inode_id, int32_t size, int block_count, int32_t *blocks) {
    int tmp_block_count;
    int last_data_block;

    inode *node = &((*vfs)->inodes[inode_id]);

    int block_count_with_indirect = get_block_count_with_indirect(block_count);

    node->nodeid = inode_id;
    node->isDirectory = 0;
    node->references = 1;
    node->file_size = size;
    node->direct1 = blocks[0]; /* First block is always used */

    last_data_block = 0;
    if (block_count > 1) {
        node->direct2 = blocks[1];
        last_data_block = 1;
    }

    if (block_count > 2) {
        node->direct3 = blocks[2];
        last_data_block = 2;
    }

    if (block_count > 3) {
        node->direct4 = blocks[3];
        last_data_block = 3;
    }

    if (block_count > 4) {
        node->direct5 = blocks[4];
        last_data_block = 4; /* This is the last data block. If we have partial data, we store them always in last direct block */
    }

    if (block_count > 5) {
        node->indirect1 = blocks[block_count_with_indirect - 1]; /* Write address of first indirect block to indirect1 */
    }

    if (block_count > (CLUSTER_SIZE / sizeof(int32_t) + 5)) {
        node->indirect2 = blocks[block_count_with_indirect - 2]; /* Use second to last block to write indirect 2*/
        seek_data_cluster(vfs, node->indirect1);
        write_vfs(vfs, &blocks[5], sizeof(int32_t), CLUSTER_SIZE / 4);

        tmp_block_count = block_count - (CLUSTER_SIZE / sizeof(int32_t) + 5);
        seek_data_cluster(vfs, node->indirect2);
        write_vfs(vfs, &blocks[(CLUSTER_SIZE / sizeof(int32_t) + 5)], sizeof(int32_t), tmp_block_count);
    }
    else  {
        tmp_block_count = block_count - 5;
        seek_data_cluster(vfs, node->indirect1);
        write_vfs(vfs,&blocks[5], sizeof(int32_t), tmp_block_count);
    }

    return last_data_block;
}

/*
 * Writes inode with given id to file
 */
void write_inode_to_vfs(VFS **vfs, int id) {
    seek_set(vfs, (*vfs)->superblock->inode_start_address + id * INODE_SIZE);

    int32_write_vfs(vfs, &((*vfs)->inodes[id].nodeid));
    int8_write_vfs(vfs, &((*vfs)->inodes[id].isDirectory));
    int8_write_vfs(vfs, &((*vfs)->inodes[id].references));
    int32_write_vfs(vfs, &((*vfs)->inodes[id].file_size));
    int32_write_vfs(vfs, &((*vfs)->inodes[id].direct1));
    int32_write_vfs(vfs, &((*vfs)->inodes[id].direct2));
    int32_write_vfs(vfs, &((*vfs)->inodes[id].direct3));
    int32_write_vfs(vfs, &((*vfs)->inodes[id].direct4));
    int32_write_vfs(vfs, &((*vfs)->inodes[id].direct5));
    int32_write_vfs(vfs, &((*vfs)->inodes[id].indirect1));
    int32_write_vfs(vfs, &((*vfs)->inodes[id].indirect2));

    flush_vfs(vfs);
}


/*
 * Flushes VFS file
 */
void flush_vfs(VFS **vfs) {
    fflush((*vfs)->vfs_file);
}

/*
 * Seeks data cluster with given number
 */
int seek_data_cluster(VFS **vfs, int block_number) {
    return seek_set(vfs, (*vfs)->superblock->data_start_address + block_number * CLUSTER_SIZE);
}

/*
 * Seeks from start of VFS file
 */
int seek_set(VFS **vfs, long int offset) {
    return fseek((*vfs)->vfs_file, offset, SEEK_SET);
}

/*
 * Seeks from current pointer of VFS file
 */
int seek_cur(VFS **vfs, long int offset) {
    return fseek((*vfs)->vfs_file, offset, SEEK_CUR);
}

/*
 * Rewinds VFS file back to beginning
 */
void rewind_vfs(VFS **vfs) {
    rewind((*vfs)->vfs_file);
}

/*
 * Writes given data to the VFS file
 */
size_t write_vfs(VFS **vfs, const void * ptr, size_t size, size_t count) {
    return fwrite(ptr, size, count, (*vfs)->vfs_file);
}

/*
 * Writes int_8 to VFS file
 */
size_t int8_write_vfs(VFS **vfs, const void * ptr) {
    return write_vfs(vfs, ptr, sizeof(int8_t), 1);
}

/*
 * Writes int_32 to VFS file
 */
size_t int32_write_vfs(VFS **vfs, const void * ptr) {
    return write_vfs(vfs, ptr, sizeof(int32_t), 1);
}

/*
 * Reads data from the VFS file on given address
 */
size_t read_vfs(VFS **vfs, void * ptr, size_t size, size_t count) {
    return fread(ptr, size, count, (*vfs)->vfs_file);
}

/*
 * Reads int_8 from the VFS file on given address
 */
size_t int8_read_vfs ( VFS **vfs, void * ptr) {
    return read_vfs(vfs, ptr, sizeof(int32_t), 1);
}

/*
 * Reads int_32 from the VFS file on given address
 */
size_t int32_read_vfs ( VFS **vfs, void * ptr) {
    return read_vfs(vfs, ptr, sizeof(int32_t), 1);
}