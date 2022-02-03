//
// Created by Stepan Mocjak on 23.01.2021.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "constants.h"
#include "fs.h"

/**
 * @brief buffer na 
 * 
 */
char command_buffer[BUFFER_SIZE];

void print_result(int result) {

    printf("RESULT: %d", result);

    switch (result) {
        case 0:
            break;
        default:
            break;
    }
}


int parse_command(char *input) {
    char *keyword;

    keyword = strtok(input, " ");

    if (!strcmp(keyword, "format")) {
        printf("FORMATTING!\n");
        int result;
        char *str_size;
        long size;

        str_size = strtok(NULL, " ");
        if (str_size != NULL) {
            size = strtol(str_size, NULL, 10);
            printf("Size: %d\n", size);

            result = format_fs(size);

            printf("FORMATTING DONE!\n");

            return result;
        } else {
            printf("format <size>\n");
        }
    } else if (!strcmp(keyword, "pwd")) {
        printf("%s\n", current_path);
    } else if (!strcmp(keyword, "cd")) {
        int result;
        char *path = strtok(NULL, " ");

        if (path != NULL) {
            result = change_dir(path);

            return result;
        } else {
            printf("cd <path>\n");
        }
    } else if (!strcmp(keyword, "mkdir")) {
//        printf("NEW DIR!\n");
        int result;
        char *path = strtok(NULL, " ");

        if (path != NULL) {
            result = create_dir(path);
//            printf(" DONE!\n");

            return result;
        } else {
            printf("mkdir <path>\n");
        }
    } else if (!strcmp(keyword, "rmdir")) {
        int result;
        char *path = strtok(NULL, " ");

        if (path != NULL) {
            result = remove_dir(path);
            return result;
        } else {
            printf("rmdir <path>\n");
        }
    } else if (!strcmp(keyword, "ls")) {
        int result;
        char *path = strtok(NULL, " ");

        result = print_dir(path);
        return result;
    } else if (!strcmp(keyword, "cp")){
        int result;
        char *file = strtok(NULL, " ");
        char *path = strtok(NULL, " ");

        if (path != NULL || file != NULL) {
            result = copy_file(file, path);

            return result;
        } else {
            printf("cp <path to copied file> <path to file destination>\n");
        }
    } else if (!strcmp(keyword, "mv")){
        int result;
        char *file = strtok(NULL, " ");
        char *path = strtok(NULL, " ");

        if (path != NULL || file != NULL) {
            result = move_file(file, path);

            return result;
        } else {
            printf("mv <path to moved file> <path to file destination>\n");
        }
    } else if (!strcmp(keyword, "rm")){
        int result;
        char *path = strtok(NULL, " ");

        if (path != NULL) {
            result = remove_file(path);

            return result;
        } else {
            printf("rm <path file>\n");
        }
    } else if (!strcmp(keyword, "incp")) {
        int result;
        char *file = strtok(NULL, " ");
        char *path = strtok(NULL, " ");

        if (path != NULL || file != NULL) {
            result = in_copy_file(file, path);
//            printf(" DONE!\n");

            return result;
        } else {
            printf("incp <path in system> <path in filesystem>\n");
        }
    } else if (!strcmp(keyword, "outcp")) {
        int result;
        char *file = strtok(NULL, " ");
        char *path = strtok(NULL, " ");

        if (path != NULL || file != NULL) {
            result = out_copy_file(file, path);
//            printf(" DONE!\n");

            return result;
        } else {
            printf("outcp <path in filesystem> <path in system>\n");
        }
    } else if (!strcmp(keyword, "cat")) {
        int result;
        char *path = strtok(NULL, " ");

        if (path != NULL) {
            result = print_file(path);

            return result;
        } else {
            printf("cat <path>\n");
        }
    } else if (!strcmp(keyword, "quit")) {
        int result = write_file_b(fs, fs_file_name);
        printf("Saving result: %d\n", result);
        return QUIT;
    }

    return 0;
}

void start_command_line() {
    memset(current_path, '\0', BUFFER_SIZE);
    memset(command_buffer, '\0', BUFFER_SIZE);
    strcpy(current_path, "/\0");
    current_inode = 1;

    while (1) {
        int result = 0;
        printf("\n%s>", current_path);
        fgets(command_buffer, BUFFER_SIZE, stdin);

        if (command_buffer[strlen(command_buffer) - 1] == '\n') {
            command_buffer[strlen(command_buffer) - 1] = '\0';
            if (command_buffer[strlen(command_buffer) - 2] == '\r') {
                command_buffer[strlen(command_buffer) - 2] = '\0';
            }
        }

        result = parse_command(command_buffer);

        print_result(result);

        fflush(stdout);

        if (result == QUIT) {
            break;
        }
    }
}