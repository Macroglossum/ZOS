/*
 * Created by Tomas Linhart on 9. 2. 2021.
 */

#ifndef SP_DIRECTORY_H
#define SP_DIRECTORY_H

#include "structures.h"
#include <stdlib.h>
#include <string.h>

dir_item *new_directory_item(int32_t inode_id, char *name);

#endif //SP_DIRECTORY_H
