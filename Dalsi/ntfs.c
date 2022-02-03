#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/limits.h>
#include "header.h"

VFS *vfs;

FILE *file_with_commands;
int is_used_file = 0;
char filename[100] = "ntfs.dat";

int main(int argc, char *argv[]) {
	
	/*
	if (argc == 2) {
		if (strcmp(argv[1], "-help") == 0) {
			help();
			return 0;
		}
		else vfs_init(&vfs, argv[1]);		
	}
	else {
		help();
		return 0;
	}
	*/

	/*
	FILE *temp_data_file = fopen(TEMP_DATA_FILENAME, "rb");
	fseek(temp_data_file, 0, SEEK_END);
	int file_size = ftell(temp_data_file);
	fseek(temp_data_file, 0, SEEK_SET);

	char c;
	fseek(temp_data_file, 14682, SEEK_SET);
	int i;
	for (i = 0; i < 2000; i++) {
		c = fgetc(temp_data_file);
		printf("%c", c);
	}
	*/

	printf("WELCOME IN PSEUDO-NTFS (iNTFS)\n\n");
	vfs_init(&vfs, filename, DISK_SIZE, 0);
	
	char command[MAX_LENGTH_OF_COMMAND];
	char *tok;

	while(1) {
		printf("\n%s%s%s ", ROOT_CHAR, vfs -> actual_path -> path, SHELL_CHAR);
		  	
		if (is_used_file == 0) {
        	    fgets(command, MAX_LENGTH_OF_COMMAND, stdin);
        	}
        	else {
        	    	fgets(command, MAX_LENGTH_OF_COMMAND, file_with_commands);
	
        		if (feof(file_with_commands) == 1) { //check end of file
        			fclose(file_with_commands);
        		        is_used_file = 0;
          		}

	            	printf("%s", command);
        	}
		tok = strtok(command, SPLIT_ARGS_CHAR);
	
		if (compare_two_string(tok, COPY_FILE) == 0) {
			copy_file(&vfs, tok);
		}
		else if (compare_two_string(tok, MOVE_FILE) == 0) {
			move_file(&vfs, tok);
		}
		else if (compare_two_string(tok, REMOVE_FILE) == 0) {
			remove_file(&vfs, tok);
		}
		else if (compare_two_string(tok, MAKE_DIRECTORY) == 0) {
			make_directory(&vfs, tok);
		}
		else if (compare_two_string(tok, REMOVE_EMPTY_DIRECTORY) == 0) {
			remove_empty_directory(&vfs, tok);
		}
		else if (compare_two_string(tok, PRINT_DIRECTORY) == 0) {
			print_directory(vfs, tok);
		}
		else if (compare_two_string(tok, PRINT_FILE) == 0) {
			print_file(vfs, tok);
		}
		else if (compare_two_string(tok, MOVE_TO_DIRECTORY) == 0) {
			move_to_directory(&vfs, tok);
		}
		else if (compare_two_string(tok, ACTUAL_DIRECTORY) == 0) {
			actual_directory(vfs);
		}
		else if (compare_two_string(tok, MFT_ITEM_INFO) == 0) {
			mft_item_info(vfs, tok);
		}
		else if (compare_two_string(tok, HD_TO_PSEUDO) == 0) {
			hd_to_pseudo(&vfs, tok);
		}
		else if (compare_two_string(tok, PSEUDO_TO_HD) == 0) {
			pseudo_to_hd(&vfs, tok);
		}
		else if (compare_two_string(tok, LOAD_COMMANDS) == 0) {
			is_used_file = load_commands(&file_with_commands, tok); 
		}
		else if (compare_two_string(tok, FILE_FORMATTING) == 0) {
			file_formatting(&vfs, tok);
		}
		else if (compare_two_string(tok, DEFRAG) == 0) {
			defrag(&vfs); //not implemented
		}
		else if (compare_two_string(tok, HELP) == 0) {
			commands_help();
		}
		else if (compare_two_string(tok, FULL_INFO) == 0) {
			full_info(vfs);
		}
		else if (compare_two_string(tok, QUIT) == 0) {
			break;
		}
		else {
			printf("Unknown command\n");
		}
	}
	printf("pseudoNTFS end\n");
}

void help() {
	printf("Help:\n\n");
	printf("First parameter (data filename): iNTFS.dat\n");
	printf("Example: ./ntfs iNTFS.dat\n");	
	return;

}
