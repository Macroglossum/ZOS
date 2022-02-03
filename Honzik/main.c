#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "fs.h"
#include "constants.h"
#include "commandline.h"
//#include "file_system.h"
//#include "command_line.h"
//#include "commands_handling.h"

char* fs = NULL;
superblock *sb = NULL;
inode *inodes = NULL;
int8_t* bitmap = NULL;
char *data_blocks = NULL;
char *fs_file_name = NULL;
int bitmap_size = 0;
int inodes_count = 0;
int items_per_block = DATA_BLOCK_SIZE / sizeof(directory_item);
int block_indexes_per_block = DATA_BLOCK_SIZE / sizeof(int32_t);
char current_path[BUFFER_SIZE];
int current_inode = 0;
int root_inode = 0;

/* prints execution hint */
void print_hint() {
    printf("Usage fs.exe <file_system_name>\n");
}

/* check program arguments */
int check_arguments(int argc) {
    if (argc < 2 || argc > 2) {
        print_hint();
        return ARGUMENTS_ERROR;
    }
    return NO_ERROR;
}

/* main function, will initialize the file system and starts the command line */
int main(int argc, char **argv) {
    int init_result;

    if (!check_arguments(argc)) {
        init_result = init_fs(argv[1]);

        if(init_result == NO_ERROR || init_result == FS_CREATED) {
            start_command_line(argv[1]);
        }
        else {
            return init_result;
        }
    }

    free(fs);
    return EXIT_SUCCESS;
}
