/*
 * Created by Tomas Linhart on 4. 2. 2021.
 */

#ifndef SP_SUPERBLOCK_H
#define SP_SUPERBLOCK_H

#include <stdint-gcc.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "structures.h"
#include "constants.h"

superblock *superblock_init(int32_t disk_size);

#endif //SP_SUPERBLOCK_H
