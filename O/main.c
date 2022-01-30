#include <stdio.h>
#include "utils.h"
#include "commands.h"
#include "structures.h"
#include "vfs.h"

VFS *current_vfs;

/*
 * Loop, that receives input and parses it and executes command
 */

void start_loop() {
    while(1) {
        char *input = remove_nl(get_line());

        if(strlen(input) == 0) {
            continue;
        }

        if (process_command_line(&current_vfs, input)) {
            break;
        }
    }
}

/*
 * Main point of program
 */
int main(int argc, const char* argv[]) {
    printf("/-----------------\\\n");
    printf("|   [I-NODE FS]   |\n");
    printf("\\-----------------/\n\n");

    if (argc == 2) {
        char * filename = (char*) argv[1];

        printf("Loading file name: %s\n", filename);

        initialize_vfs(&current_vfs, filename);

        start_loop();
    } else {
        printf ("Incorrect parameters! End of program!\n\n");
        help();
    }
}