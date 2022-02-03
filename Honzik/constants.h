//
// Created by Stepan Mocjak on 23.01.2021.
//

#ifndef FS_CONST_H
#define FS_CONST_H

#define FREE 0
#define DATA_BLOCK_SIZE 512
#define INODES_RATIO 10
#define MAX_BLOCKS_PER_INODE 261
#define NAME_LEN 12
#define EMPTY -1
#define NOT_FOUND -1
#define EMPTY_BLOCK -1

#define BUFFER_SIZE 256
#define STARTING_BIT 7
#define BITS_PER_BYTE 8

#define NO_ERROR 0
#define ARGUMENTS_ERROR 1
#define FS_CREATED 2
#define FS_FORMAT_ERROR 3
#define FILE_ERROR 4
#define EXISTS 5
#define MEMORY_ERR 6
#define DIR_NOT_EMPTY 7
#define NOT_EXIST 8
#define QUIT 99

#endif //FS_CONST_H
