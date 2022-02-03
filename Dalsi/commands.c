#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include "header.h" 

void copy_file(VFS **vfs, char *tok) {

	tok = strtok(NULL, SPLIT_ARGS_CHAR);

	if (tok == NULL || strlen(tok) <= 1) {
		printf("SOURCE NOT DEFINED\n");
		return;
	}

	char source[MAX_LENGTH_OF_COMMAND];
	strcpy(source, tok);

	tok = strtok(NULL, SPLIT_ARGS_CHAR);

	if (tok == NULL) {
		printf("DESTINATION NOT DEFINED\n");
		return;
	}

	MFT_ITEM *dest_folder = NULL;

	if (strlen(tok) == 1) dest_folder = (*vfs) -> mft -> items[0];
	else dest_folder = get_mft_item_from_path((*vfs), tok);

	char destination[MAX_LENGTH_OF_COMMAND];
	strcpy(destination, tok);

	MFT_ITEM *moved_file = get_mft_item_from_path((*vfs), source);

	if (moved_file == NULL) {
		printf("FILE NOT FOUND\n");
		return;
	}
	else if (moved_file -> isDirectory) {
		printf("Moved file is DIRECTORY!\n");
		return;
	}
	
	if (dest_folder == NULL) {
		printf("PATH NOT FOUND\n");
		return;
	}
	else if (!dest_folder -> isDirectory) {
		printf("Destination directory is FILE!\n");
		return;
	}

	
	if (check_if_folder_contains_item((*vfs) -> mft, dest_folder, moved_file -> item_name)) {
		printf("Directory '%s' already contains file/directory with name '%s'\n", dest_folder -> item_name, moved_file -> item_name);
		return;	
	}

	copy_given_file(vfs, dest_folder, moved_file, destination);
}

void move_file(VFS **vfs, char *tok) {

	tok = strtok(NULL, SPLIT_ARGS_CHAR);

	if (tok == NULL || strlen(tok) <= 1) {
		printf("SOURCE NOT DEFINED\n");
		return;
	}

	char source[MAX_LENGTH_OF_COMMAND];
	strcpy(source, tok);

	tok = strtok(NULL, SPLIT_ARGS_CHAR);

	if (tok == NULL || strlen(tok) <= 1) {
		printf("DESTINATION NOT DEFINED\n");
		return;
	}

	char destination[MAX_LENGTH_OF_COMMAND];
	strcpy(destination, tok);

	MFT_ITEM *moved_file = get_mft_item_from_path((*vfs), source);
	MFT_ITEM *dest_folder = get_mft_item_from_path((*vfs), destination);

	if (moved_file == NULL) {
		printf("FILE NOT FOUND\n");
		return;
	}
	else if (moved_file -> isDirectory) {
		printf("Moved file is DIRECTORY!\n");
		return;
	}
	
	if (dest_folder == NULL) {
		printf("PATH NOT FOUND\n");
		return;
	}
	else if (!dest_folder -> isDirectory) {
		printf("Destination directory is FILE!\n");
		return;
	}

	
	if (check_if_folder_contains_item((*vfs) -> mft, dest_folder, moved_file -> item_name)) {
		printf("Directory '%s' already contains file/directory with name '%s'\n", dest_folder -> item_name, moved_file -> item_name);
		return;	
	}

	(*vfs) -> mft -> items[moved_file -> uid] -> parentID = dest_folder -> uid;
	printf("File '%s' was moved to directory '%s'\n", moved_file -> item_name, dest_folder -> item_name);
}

void remove_file(VFS **vfs, char *tok) {

	tok = strtok(NULL, SPLIT_ARGS_CHAR);

	if (tok == NULL || strlen(tok) <= 1) {
		printf("FILE NOT DEFINED\n");
		return;
	}

	MFT_ITEM *removed_file = get_mft_item_from_path((*vfs), tok);

	if (removed_file == NULL) {
		printf("FILE NOT FOUND\n");
		return;
	}
	else if (removed_file -> isDirectory) {
		printf("Moved file is DIRECTORY!\n");
		return;
	}

	printf("File '%s' was successfully removed\n", removed_file -> item_name); 
	remove_given_file(vfs, removed_file);
}

void make_directory(VFS **vfs, char *tok) {

	tok = strtok(NULL, SPLIT_ARGS_CHAR);
	if (tok == NULL || strlen(tok) <= 1) {
		printf("NAME OF DIRECTORY NOT DEFINED\n");
		return;
	}

	MFT_ITEM *item = get_mft_item_from_path((*vfs), tok);
	if (item == NULL) {
		
		int folders_count = 0;
		char path[strlen(tok)];
		strcpy(path, tok);
		char *ttok = strtok(path, "/");
		while(ttok != NULL) {
			folders_count++;
			ttok = strtok(NULL, "/");
		}

		char cutted_path[PATH_MAX];
		strcpy(cutted_path, (*vfs) -> actual_path -> path);
		if (strlen((*vfs) -> actual_path -> path) > 0) strcat(cutted_path, "/");
		tok = strtok(tok, "/");
		int i;
		for (i = 0; i < folders_count - 1; i++) {
			strcat(cutted_path, tok);
			tok = strtok(NULL, "/");
			if (i < folders_count - 2) strcat(cutted_path, "/");
		}

		if (strlen(cutted_path) == 0) {
			mft_item_init(vfs, (*vfs) -> mft -> size, 0, tok, 1, DIRECTORY_SIZE);
		}
		else {
			int parentID = find_folder_id((*vfs) -> mft, cutted_path);
			if (parentID == -1) {
				printf("PATH NOT FOUND\n");
			}
			else {
				mft_item_init(vfs, (*vfs) -> mft -> size, parentID, tok, 1, DIRECTORY_SIZE);
			}
		}
		printf("Directory '%s' CREATED\n", tok);
	}
	else {
		printf("Directory '%s' already EXISTS!\n", item -> item_name);
		return;
	}
}

void remove_empty_directory(VFS **vfs, char *tok) {

	tok = strtok(NULL, SPLIT_ARGS_CHAR);
	if (tok == NULL || strlen(tok) <= 1) {
		printf("NAME OF DIRECTORY NOT DEFINED\n");
		return;
	}

	MFT_ITEM *item = get_mft_item_from_path((*vfs), tok);
	if (item == NULL) {
		printf("DIRECTORY NOT FOUND\n");
		return;
	}
	else {
		if (!item -> isDirectory) {
			printf("ITEM IS FILE\n");
			return;
		}
		if (is_folder_empty((*vfs) -> mft, item -> uid) == 0) {
			printf("DIRECTORY '%s' REMOVED\n", item -> item_name);
			remove_directory(vfs, item -> uid);
		}
		else {
			printf("DIRECTORY '%s' IS NOT EMPTY\n", item -> item_name);
			return;
		}
	}
}

void print_directory(VFS *vfs, char *tok) {

	MFT_ITEM *item = NULL;
	int folder_ID = -1;

	tok = strtok(NULL, SPLIT_ARGS_CHAR);
	
	if (tok == NULL || strlen(tok) <= 1) {
		if (strlen(vfs -> actual_path -> path) == 0) {
			folder_ID = 0;
			item = vfs -> mft -> items[0];
		}
		else {
			folder_ID = find_folder_id(vfs -> mft, vfs -> actual_path -> path);
			item = find_mft_item_by_uid(vfs -> mft, folder_ID);
		}
		
		if (item != NULL) {
			if (item -> isDirectory) {
				print_folder_content(vfs -> mft, folder_ID);
			}
			else {
				printf("ITEM IS FILE\n");
			}
		}
		else printf("PATH NOT FOUND\n");
	}
	else {		
		item = get_mft_item_from_path(vfs, tok);

		if (item != NULL) {
			if (item -> isDirectory) {
				print_folder_content(vfs -> mft, item -> uid);
			}
			else {
				printf("ITEM IS FILE\n");
			}
		}
		else printf("PATH NOT FOUND\n");
	}
}

void print_file(VFS *vfs, char *tok) {

	tok = strtok(NULL, SPLIT_ARGS_CHAR);

	if (tok == NULL || strlen(tok) <= 1) {
		printf("FILENAME NOT DEFINED\n");
		return;
	}

	MFT_ITEM *item = get_mft_item_from_path(vfs, tok);

	if (item != NULL) {
		if (!item -> isDirectory) {
			print_file_content(vfs, item);
		}
		else {
			printf("ITEM IS DIRECTORY\n");
		}
	}
	else printf("FILE NOT FOUND\n");
}

void move_to_directory(VFS **vfs, char *tok) {
	MFT_ITEM *item = NULL;
	tok = strtok(NULL, SPLIT_ARGS_CHAR);

	if (tok != NULL) {
		if (strlen(tok) > 1) {
			if (strncmp(tok, "..", 2) == 0) {				
				if (strlen((*vfs) -> actual_path -> path) > 1) {
					go_to_parent_folder(vfs);
				}
			}
			else {				
				if (strlen(tok) > 0 && tok[strlen(tok) - 1] == '\n') tok[strlen(tok) - 1] = '\0';
				char temp_path[strlen((*vfs) -> actual_path -> path) + strlen(tok)];
				strcpy(temp_path, (*vfs) -> actual_path -> path); 
				
				if (tok[0] != 47) strcat(temp_path, "/");
				strcat(temp_path, tok);

				int folder_ID = find_folder_id((*vfs) -> mft, temp_path);		
				item = find_mft_item_by_uid((*vfs) -> mft, folder_ID);
				
				if (item != NULL) {
				
					if (!item -> isDirectory) {
						printf("ITEM IS FILE\n");
						return;
					}
	
					set_path_to_root(vfs);	
					if (tok[strlen(tok) - 1] == 47) strncat((*vfs) -> actual_path -> path, temp_path, strlen(temp_path) - 1);
					else strncat((*vfs) -> actual_path -> path, temp_path, strlen(temp_path));
									
				}
				else printf("PATH NOT FOUND\n");	
			}
			return;
		}
		else {
			printf("NAME OF DIRECTORY NOT DEFINED\n");
			return;
		}
	}
	else {
		set_path_to_root(vfs);	
		return;
	}
}

void actual_directory(VFS *vfs) {
	
	char path[strlen(vfs -> actual_path -> path)];
	if (strlen(vfs -> actual_path -> path) > 0) strcpy(path, vfs -> actual_path -> path);
	else strcpy(path, "/");
	printf("%s", path);
}

void mft_item_info(VFS *vfs, char *tok) {

	tok = strtok(NULL, SPLIT_ARGS_CHAR);

	if (tok == NULL || strlen(tok) <= 1) {
		printf("SOURCE NAME NOT DEFINED\n");
		return;
	}
	
	MFT_ITEM *item = get_mft_item_from_path(vfs, tok);

	if (item == NULL) {
		printf("FILE NOT FOUND\n");
		return;
	}

	printf("NAME: %s\n", item -> item_name);
	printf("UID: %d\n", item -> uid);
	printf("PUID: %d\n", item -> parentID);
	printf("SIZE: %li\n", item -> item_size);
	printf("FRAGMENTS: Count - %d\n", item -> fragments_created);	
	int i;
	for (i = 0; i < item -> fragments_created; i++) {	
		printf("FRAGMENT %d: Location - %d\n", i, item -> fragment_start_address[i]);
		printf("	CLUSTERS: START_ID - %d; Count - %d\n", item -> start_cluster_ID[i], item -> fragment_count[i]);
	}
}

void hd_to_pseudo(VFS **vfs, char *tok) {

	tok = strtok(NULL, SPLIT_ARGS_CHAR);

	if (tok == NULL || strlen(tok) <= 0) {
		printf("SOURCE NOT DEFINED\n");
		return;
	}

	char source[MAX_LENGTH_OF_COMMAND];
	strcpy(source, tok);

	tok = strtok(NULL, SPLIT_ARGS_CHAR);
	if (tok == NULL) {
		printf("DESTINATION NOT DEFINED\n");
		return;
	}

	FILE *file_src = fopen(source, "rb");
	if (file_src == NULL) {
		printf("FILE NOT FOUND\n");
		return;
	}

	MFT_ITEM *dest = NULL;

	if (strlen(tok) == 1 || (strlen(tok) == 2 && tok[0] == 46)) dest = (*vfs) -> mft -> items[0];
	else dest = get_mft_item_from_path((*vfs), tok);	 

	if (dest == NULL) {
		printf("PATH NOT FOUND\n");
		return;
	}

	create_file_from_FILE(vfs, file_src, source, dest);
}

void pseudo_to_hd(VFS **vfs, char *tok) {
	char source[PATH_MAX];
	char dest[PATH_MAX];
	
	tok = strtok(NULL, SPLIT_ARGS_CHAR);
	if (tok == NULL || strlen(tok) <= 1) {
		printf("SOURCE NOT DEFINED\n");
		return;
	}
	strcpy(source, tok);

	tok = strtok(NULL, SPLIT_ARGS_CHAR);	

	MFT_ITEM *item_source = get_mft_item_from_path((*vfs), source);

	if (item_source == NULL) {
		printf("FILE NOT FOUND\n");
		return;
	}

	if (item_source -> isDirectory) {
		printf("Can't outcp directory!\n");
		return;
	}

	if (tok == NULL) {
		printf("DESTINATION NOT DEFINED\n");
		return;
	}
	if (strlen(tok) == 1 || (strlen(tok) == 2 && tok[0] == 46)) tok[0] = '\0';
	strcpy(dest, tok);

	if (strlen(dest) > 0 && dest[strlen(dest) - 1] == '\n') dest[strlen(dest) - 1] = '\0';	

	char full_path[PATH_MAX];
	strcpy(full_path, dest);
	if(strlen(dest) > 0) strcat(full_path, "/");
	strcat(full_path, item_source -> item_name);

	if (strlen(dest) == 0 || directory_exists(dest)) {
		FILE *file_dest = fopen(full_path, "wb");
		
		if (item_source -> item_size > 0) {
			int i, j;
			int position = 0;
			char buffer[CLUSTER_SIZE];
			int32_t actual_size = item_source -> item_size;
			for (i = 0; i < item_source -> fragments_created; i++) {
				for (j = 0; j < item_source -> fragment_count[i]; j++) {
					fseek((*vfs) -> FILE, (*vfs) -> boot_record -> data_start_address + 1 + CLUSTER_SIZE*(item_source -> start_cluster_ID[i] + j), SEEK_SET); 
					fread(buffer, CLUSTER_SIZE, 1, (*vfs) -> FILE);	

					fseek(file_dest, position * CLUSTER_SIZE, SEEK_SET);
					position++; 
					if (actual_size >= CLUSTER_SIZE) {
						actual_size -= CLUSTER_SIZE;
						fwrite(buffer, CLUSTER_SIZE, 1, file_dest); 
					}
					else {
						fwrite(buffer, actual_size - 1, 1, file_dest); 
						fflush(file_dest);
						fputc(0x0a, file_dest); //LF
						break;
					}
					fflush(file_dest);
				}
			}
		}
		fflush(file_dest);
		fclose(file_dest);
		printf("File '%s' SUCCESSFULLY COPIED INTO HD\n", item_source -> item_name);
	}
	else {
		printf("PATH NOT FOUND\n");
		return;
	}
}

int load_commands(FILE **file_with_commands, char *tok) {
	
	tok = strtok(NULL, " \n");
	(*file_with_commands) = fopen(tok, "r");
	if ((*file_with_commands) != NULL) {
		printf("OK\n");       
		return 1;
	}
        else {
		printf("FILE NOT FOUND\n");
		return 0;
	}
}

void file_formatting(VFS **vfs, char *tok) {

	tok = strtok(NULL, SPLIT_ARGS_CHAR);

	int last_digit_index = index_of_last_digit(tok);
	
	char number[last_digit_index + 1];
	memset(number, 0, last_digit_index + 1);
	strncpy(number, tok, last_digit_index);

	int multiple_size = (strlen(tok) - last_digit_index) - 1;
	
	char multiple[multiple_size];

	int i = 0;
	int j = 0;

	for (i = last_digit_index; i < strlen(tok) - 1; i++) {
		multiple[j] = tok[i];
		j++;
	}

	int multiple_number = get_multiple(multiple, multiple_size);

	int disk_size = my_atoi(number) * multiple_number;
	vfs_init(vfs, (*vfs) -> filename, disk_size, 1);
	printf("Data file with name %s was formated. Disk size = %d\n", (*vfs) -> filename, disk_size);
}

void defrag(VFS **vfs) {
	printf("Starting defragmentation\n");

	defrag_copy_data_temp_file(vfs);
	defrag_bitmap(vfs);

	(*vfs) -> FILE = fopen((*vfs) -> filename, "r+");
	fseek((*vfs) -> FILE, 0, SEEK_SET);    
	fwrite((*vfs) -> boot_record, sizeof(BOOT_RECORD), 1, (*vfs) -> FILE);
	fflush((*vfs) -> FILE);
	fwrite_mft(vfs);
	fwrite_bitmap(vfs);
	fflush((*vfs) -> FILE);
	
	defrag_init_mft_items(vfs);
	remove(TEMP_DATA_FILENAME);

	printf("Defragmentation ends successfully\n");
}

void full_info(VFS *vfs) {
	print_vfs(vfs);
}

void commands_help() {
	printf("Commands:\n");
	printf("%s - Copy file (%s s1 s2)\n", COPY_FILE, COPY_FILE);
	printf("%s - Move file (%s s1 s2)\n", MOVE_FILE, MOVE_FILE);
	printf("%s - Remove file (%s s1)\n", REMOVE_FILE, REMOVE_FILE);
	printf("%s - Make directory (%s s1)\n", MAKE_DIRECTORY, MAKE_DIRECTORY);
	printf("%s - Remove empty directory (%s s1)\n", REMOVE_EMPTY_DIRECTORY, REMOVE_EMPTY_DIRECTORY);
	printf("%s - Print directory (%s s1)\n", PRINT_DIRECTORY, PRINT_DIRECTORY);
	printf("%s - Print file (%s s1)\n", PRINT_FILE, PRINT_FILE);
	printf("%s - Move to directory (%s s1)\n", MOVE_TO_DIRECTORY, MOVE_TO_DIRECTORY);
	printf("%s - Actual directory (%s)\n", ACTUAL_DIRECTORY, ACTUAL_DIRECTORY);
	printf("%s - MFT Item info (%s s1)\n", MFT_ITEM_INFO, MFT_ITEM_INFO);
	printf("%s - Copy file from HD to pseudoNTFS (%s s1 s2)\n", HD_TO_PSEUDO, HD_TO_PSEUDO);
	printf("%s - Copy file from pseudoNTFS to HD (%s s1 s2)\n", PSEUDO_TO_HD, PSEUDO_TO_HD);
	printf("%s - Load commands from file (%s s1)\n", LOAD_COMMANDS, LOAD_COMMANDS);
	printf("%s - Formatting file (%s 10MB)\n", FILE_FORMATTING, FILE_FORMATTING);
	printf("%s - Defragmentation (%s)\n", DEFRAG, DEFRAG);
	printf("%s - Info about pseudoNTFS (%s)\n", FULL_INFO, FULL_INFO);
	printf("%s - Quit pseudoNTFS (%s)\n", QUIT, QUIT);
	printf("%s - Available commands (%s)\n", HELP, HELP);
}
