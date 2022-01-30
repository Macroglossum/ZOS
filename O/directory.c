/*
 * Created by Tomas Linhart on 9. 2. 2021.
 */
#include "directory.h"

/*
 * Creates new directory item
 */
dir_item *new_directory_item(int32_t inode_id, char *name) {
    char buff[12] = {'\0'};

    dir_item *d_item = calloc(1, sizeof(dir_item));

    strncpy(buff, name, strlen(name));
    d_item->inode = inode_id;
    strncpy(d_item->item_name, buff, 12);
    d_item->next = NULL;

    return d_item;
}

