#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include "header.h"


void mft_init(VFS **vfs) {
	(*vfs) -> mft = calloc(1, sizeof(MFT));
	(*vfs) -> mft -> size = 0;
	(*vfs) -> mft -> items = calloc((*vfs) -> mft -> size, sizeof(MFT_ITEM));

	mft_item_init(vfs, (*vfs) -> mft -> size, -1, ROOT_NAME, 1, DIRECTORY_SIZE);
}

int mft_item_init(VFS **vfs, int uid, int parentID, char *name, int isDirectory, int item_size) {
	if (bitmap_contains_free_cluster((*vfs) -> bitmap) == 1) {
		printf("Out of memory, cannot create new files\n");
		return 0;
	} 

	(*vfs) -> mft -> items = realloc((*vfs) -> mft -> items, ((*vfs) -> mft -> size + 1) * sizeof(MFT_ITEM));

	(*vfs) -> mft -> items[(*vfs) -> mft -> size] = calloc(1, sizeof(MFT_ITEM));
	(*vfs) -> mft -> items[(*vfs) -> mft -> size] -> uid = uid;
	(*vfs) -> mft -> items[(*vfs) -> mft -> size] -> parentID = parentID;
	(*vfs) -> mft -> items[(*vfs) -> mft -> size] -> isDirectory = isDirectory;                           
	(*vfs) -> mft -> items[(*vfs) -> mft -> size] -> item_order = 1;                        
	(*vfs) -> mft -> items[(*vfs) -> mft -> size] -> item_order_total = 1;              
	(*vfs) -> mft -> items[(*vfs) -> mft -> size] -> item_size = item_size;        
	strcpy((*vfs) -> mft -> items[(*vfs) -> mft -> size] -> item_name, name);
	(*vfs) -> mft -> items[(*vfs) -> mft -> size] -> fragments_created = 0;
	int cluster_count = 1;
	if (item_size != 0) {
		cluster_count = item_size / CLUSTER_SIZE;
		if ((item_size % CLUSTER_SIZE) != 0) cluster_count++;
	}
	int success = mft_fragment_init(vfs, cluster_count);		
	if (success) {
		(*vfs) -> mft -> size++;

		fwrite_mft(vfs);
		fwrite_mft_item(vfs, uid);
		return 1;
	}
	else return 0;
}

int mft_fragment_init(VFS **vfs, int cluster_count) {
	int i = 0;	
	int already_setted = 0;
	while(1) {
		if (i >= MFT_FRAGMENTS_COUNT) {
			printf("FILE TOO LARGE\n");
			break;
		}

		struct the_fragment_temp *temp = find_free_cluster(&(*vfs) -> bitmap, cluster_count - already_setted);	
		if (temp -> start_cluster_ID == -1 && temp -> count == 0 && temp -> successful == 0) break;
		already_setted += temp -> count;
	
		(*vfs) -> mft -> items[(*vfs) -> mft -> size] -> fragment_count[i] = temp -> count;
		(*vfs) -> mft -> items[(*vfs) -> mft -> size] -> start_cluster_ID[i] = temp -> start_cluster_ID;
		(*vfs) -> mft -> items[(*vfs) -> mft -> size] -> fragment_start_address[i] = (*vfs) -> boot_record -> data_start_address + 1 + temp -> start_cluster_ID * CLUSTER_SIZE;

		(*vfs) -> mft -> items[(*vfs) -> mft -> size] -> fragments_created++;
		if (temp -> successful == 1) break; 

		i++;
	}

	if (cluster_count != already_setted) {

		for (i = 0; i < (*vfs) -> mft -> items[(*vfs) -> mft -> size] -> fragments_created; i++) {
			int from = (*vfs) -> mft -> items[(*vfs) -> mft -> size] -> start_cluster_ID[i];
			int count = (*vfs) -> mft -> items[(*vfs) -> mft -> size] -> fragment_count[i];

			int j;
			for (j = 0; j < count; j++) {
				(*vfs) -> bitmap -> data[from + j] = 0;
			}
		}
		printf("Out of memory, cannot create new files\n");

		if ((*vfs) -> mft -> items[(*vfs) -> mft -> size] -> isDirectory) {
			remove_directory(vfs, (*vfs) -> mft -> items[(*vfs) -> mft -> size] -> uid);
		}
		else {
			remove_given_file(vfs, (*vfs) -> mft -> items[(*vfs) -> mft -> size]);
		}		
		return 0;
	}

	fwrite_bitmap(vfs);
	return 1;
}

void fread_mft(VFS **vfs, FILE *file) {
	(*vfs) -> mft = calloc(1, sizeof(MFT));
	fseek(file, (*vfs) -> boot_record -> mft_start_address, SEEK_SET);
	fread((*vfs) -> mft, sizeof(MFT), 1, file);
	
	(*vfs) -> mft -> items = calloc((*vfs) -> mft -> size, sizeof(MFT_ITEM));
	int i;
	for (i = 0; i < (*vfs) -> mft -> size; i++) {
		(*vfs) -> mft -> items[i] = calloc(1, sizeof(MFT_ITEM));
		fseek(file, (*vfs) -> boot_record -> mft_start_address + sizeof(MFT) + 1 + i*sizeof(MFT_ITEM), SEEK_SET);
		fread((*vfs) -> mft -> items[i], sizeof(MFT_ITEM), 1, file); 
	}
}

MFT_ITEM *find_mft_item_by_uid(MFT *mft, int uid) {
	int i;
	for (i = 0; i < mft -> size; i++) {
		if (mft -> items[i] -> uid == uid) {
			return mft -> items[i];
		}			
	}

	return NULL;
}

MFT_ITEM *get_mft_item_from_path(VFS *vfs, char *tok) {
	MFT_ITEM *item = NULL;

	if (strlen(tok) > 0 && tok[strlen(tok) - 1] == '\n') tok[strlen(tok) - 1] = '\0';
	char temp_path[strlen(vfs -> actual_path -> path) + strlen(tok)];
	strcpy(temp_path, vfs -> actual_path -> path); 
				
	if (strlen(tok) > 0 && tok[0] != 47) strcat(temp_path, "/");
	strcat(temp_path, tok);
	int folder_ID = find_folder_id(vfs -> mft, temp_path);		
	item = find_mft_item_by_uid(vfs -> mft, folder_ID);
	return item;
}

void remove_directory(VFS **vfs, int uid) {
	int i;
	for (i = 0; i < (*vfs) -> mft -> size; i++) {
		if ((*vfs) -> mft -> items[i] -> uid == uid) {	
			(*vfs) -> bitmap -> data[(*vfs) -> mft -> items[i] -> start_cluster_ID[0]] = 0; 
			if (((*vfs) -> mft -> size - 1) > 0) (*vfs) -> mft -> items[i] = (*vfs) -> mft -> items[(*vfs) -> mft -> size - 1];
			(*vfs) -> mft -> items[i] -> uid = uid;
			(*vfs) -> mft -> items = realloc((*vfs) -> mft -> items, ((*vfs) -> mft -> size - 1) * sizeof(MFT_ITEM));
			(*vfs) -> mft -> size--;


			fwrite_mft(vfs);
			fwrite_mft_item(vfs, uid);
			fwrite_bitmap(vfs);
			return;
		}
	}
}

size_t get_size_of_items(MFT *mft) {
	size_t size = 0;
	int i;
	
	for(i = 0; i < mft -> size; i++) {
		size += mft -> items[i] -> item_size;
	}

	return size;
}

int find_folder_id(MFT *mft, char *path) {
	char actual_path[strlen(path)];
	strcpy(actual_path, path);	

	int i;
	int actual_parentID = 0; //id of root
	int folder_ID = -1;
	int exit = 0;
	char *tok = strtok(actual_path, "/");
	while(1) {
		for (i = 0; i < mft -> size; i++) {
			if (strcmp(tok, mft -> items[i] -> item_name) == 0) {
				if (mft -> items[i] -> parentID == actual_parentID) {
					folder_ID = mft -> items[i] -> uid;
					actual_parentID = folder_ID;
					break;
				}
			} 
			folder_ID = -1;
		}
		if (folder_ID == -1) break;
		if (exit == 1) break;
		tok = strtok(NULL, "/");
		if (tok == NULL) break;
	}
	return folder_ID;
}

void print_folder_content(MFT *mft, int parentID) {
	int i;
	for (i = 1; i < mft -> size; i++) {		
		if (mft -> items[i] -> parentID == parentID) {
			if (mft -> items[i] -> isDirectory == 1) printf("+"); 
			else printf("-");

			printf("%s\n", mft -> items[i] -> item_name);
		}
	}
}

int is_folder_empty(MFT *mft, int folderID) {
	int i;
	for (i = 0; i < mft -> size; i++) {
		if (mft -> items[i] -> parentID == folderID) return 1;
	}
	
	return 0;
}

void fwrite_mft(VFS **vfs) {
	fseek((*vfs) -> FILE, (*vfs) -> boot_record -> mft_start_address, SEEK_SET);
	fwrite((*vfs) -> mft, sizeof(MFT), 1, (*vfs) -> FILE);
	fflush((*vfs) -> FILE);
}

void fwrite_mft_item(VFS **vfs, int uid) {
	fseek((*vfs) -> FILE, (*vfs) -> boot_record -> mft_start_address + sizeof(MFT) + 1 + uid*sizeof(MFT_ITEM), SEEK_SET);
	fwrite((*vfs) -> mft -> items[uid], sizeof(MFT_ITEM), 1, (*vfs) -> FILE);
	fflush((*vfs) -> FILE);
}

int check_if_folder_contains_item(MFT *mft, MFT_ITEM *folder, char *item_name) {
	int i;
	for (i = 0; i < mft -> size; i++) {
		if (!compare_two_string(mft -> items[i] -> item_name, item_name)) {
			if (mft -> items[i] -> parentID == folder -> uid) return 1;
		} 
	}
	return 0;
}

void create_file_from_FILE(VFS **vfs, FILE *source, char *source_name, MFT_ITEM *dest) {
	fseek(source, 0, SEEK_END);
	int file_size = ftell(source);
	fseek(source, 0, SEEK_SET);

	char filename[PATH_MAX];
	char *tok = strtok(source_name, "/");
	while(tok != NULL) {
		strcpy(filename, tok);
		tok = strtok(NULL, "/");
	}

	if (check_if_folder_contains_item((*vfs) -> mft, dest, filename)) {
		printf("File or directory already exists with name '%s'\n", filename);
		return;
	}

	int success = mft_item_init(vfs, (*vfs) -> mft -> size, dest -> uid, filename, 0, file_size);

	if (success) {
		printf("File '%s' CREATED\n", filename);
	}

	int i;

	MFT_ITEM *item = find_mft_item_by_uid((*vfs) -> mft, (*vfs) -> mft -> size - 1);

	int file_part = 1;
	if (file_size != 0) {
		file_part = file_size / CLUSTER_SIZE;
		if (file_size % CLUSTER_SIZE != 0) file_part++;
	}


	int read_size = CLUSTER_SIZE;
	char buffer[file_part][read_size];

	for (i = 0; i < file_part; i++) {
		fseek(source, i * read_size, SEEK_SET);
		fread(buffer[i], read_size, 1, source);
		if (i == (file_part - 1)) buffer[i][strlen(buffer[i]) - 1] = '\0';
	}

	int position = 0;
	int plus_cluster = 0;
	long actual_size = item -> item_size;
	for (i = 0; i < file_part; i++) {
		fseek((*vfs) -> FILE, (*vfs) -> boot_record -> data_start_address + 1 + CLUSTER_SIZE*(item -> start_cluster_ID[position] + plus_cluster), SEEK_SET);
		plus_cluster++;
		if (actual_size >= CLUSTER_SIZE) {
			actual_size -= CLUSTER_SIZE;
			fwrite(buffer[i], CLUSTER_SIZE, 1, (*vfs) -> FILE); 
		}
		else {
			fwrite(buffer[i], actual_size, 1, (*vfs) -> FILE); 
			break;
		}
		if (plus_cluster == item -> fragment_count[position]) {
			position++;
			plus_cluster = 0;
		}
		fflush((*vfs) -> FILE);
	}
}

void remove_given_file(VFS **vfs, MFT_ITEM *file) {
	int i, j, k;
	for (i = 0; i < (*vfs) -> mft -> size; i++) {
		if ((*vfs) -> mft -> items[i] -> uid == file -> uid) {	
			(*vfs) -> bitmap -> data[(*vfs) -> mft -> items[i] -> start_cluster_ID[0]] = 0; 

			for (j = 0; j < (*vfs) -> mft -> items[i] -> fragments_created; j++) {
				for (k = 0; k < (*vfs) -> mft -> items[i] -> fragment_count[j]; k++) {
					(*vfs) -> bitmap -> data[(*vfs) -> mft -> items[i] -> start_cluster_ID[j] + k] = 0;
				}
			}

			if (((*vfs) -> mft -> size - 1) > 0) (*vfs) -> mft -> items[i] = (*vfs) -> mft -> items[(*vfs) -> mft -> size - 1];
			(*vfs) -> mft -> items[i] -> uid = file -> uid;
			(*vfs) -> mft -> items = realloc((*vfs) -> mft -> items, ((*vfs) -> mft -> size - 1) * sizeof(MFT_ITEM));
			(*vfs) -> mft -> size--;


			fwrite_mft(vfs);
			fwrite_mft_item(vfs, file -> uid);
			fwrite_bitmap(vfs);
			return;
		}
	}
}

void print_file_content(VFS *vfs, MFT_ITEM *item) {

	if (item -> item_size == 0) {
		printf("\n");
		return;
	}
	int file_part = item -> item_size / CLUSTER_SIZE;
	if (item -> item_size % CLUSTER_SIZE != 0) file_part++;

	char buffer[CLUSTER_SIZE];

	int i, j;
	int actual_size = item -> item_size;
	for (i = 0; i < item -> fragments_created; i++) {
		for (j = 0; j < item -> fragment_count[i]; j++) {
			fseek(vfs -> FILE, vfs -> boot_record -> data_start_address + 1 + CLUSTER_SIZE*(item -> start_cluster_ID[i] + j), SEEK_SET);
			if (actual_size >= CLUSTER_SIZE) { 
				fread(buffer, CLUSTER_SIZE, 1, vfs -> FILE); 
				printf("%s", buffer);
				actual_size -= CLUSTER_SIZE;
			}
			else {
				fread(buffer, actual_size, 1, vfs -> FILE);
				int k;
				for (k = 0; k < actual_size; k++) {							
					printf("%c", buffer[k]);
				}
				break;
			}
		}
	}
	printf("\n");
}

void copy_given_file(VFS **vfs, MFT_ITEM *dest_folder, MFT_ITEM *file, char *destination) {
	int success = mft_item_init(vfs, (*vfs) -> mft -> size, dest_folder -> uid, file -> item_name, 0, file -> item_size);
	
	MFT_ITEM *copied_file = find_mft_item_by_uid((*vfs) -> mft, (*vfs) -> mft -> size - 1);
	if (success) {
		if (file -> item_size > 0) {
			int i, j;
			char buffer[CLUSTER_SIZE];
			long actual_size = file -> item_size;
			for (i = 0; i < file -> fragments_created; i++) {
				for (j = 0; j < file -> fragment_count[i]; j++) {
					fseek((*vfs) -> FILE, (*vfs) -> boot_record -> data_start_address + 1 + CLUSTER_SIZE*(file -> start_cluster_ID[i] + j), SEEK_SET); 
					fread(buffer, CLUSTER_SIZE, 1, (*vfs) -> FILE);	

					fseek((*vfs) -> FILE, (*vfs) -> boot_record -> data_start_address + 1 + CLUSTER_SIZE*(copied_file -> start_cluster_ID[i] + j), SEEK_SET); 
					if (actual_size >= CLUSTER_SIZE) {
						actual_size -= CLUSTER_SIZE;
						fwrite(buffer, CLUSTER_SIZE, 1, (*vfs) -> FILE); 
					}
					else {
						fwrite(buffer, actual_size, 1, (*vfs) -> FILE); 
						break;
					}
					fflush((*vfs) -> FILE);
				}
			}
		}
		fflush((*vfs) -> FILE);		
		printf("File '%s' copied into destination '%s'\n", file -> item_name, destination);
	}
}

void defrag_copy_data_temp_file(VFS **vfs) {
	int i, j, k;
	FILE *temp_data_file = fopen(TEMP_DATA_FILENAME, "wb");
	
	char buffer[CLUSTER_SIZE];
	long actual_size = 0;

	fseek(temp_data_file, 0, SEEK_SET);
	fwrite((*vfs) -> boot_record, sizeof(BOOT_RECORD), 1, temp_data_file);
	fflush(temp_data_file);

	fseek(temp_data_file, (*vfs) -> boot_record -> mft_start_address, SEEK_SET);
	fwrite((*vfs) -> mft, sizeof(MFT), 1, temp_data_file);
	fflush(temp_data_file);

	for (i = 0; i < (*vfs) -> mft -> size; i++) {
		fseek(temp_data_file, (*vfs) -> boot_record -> mft_start_address + sizeof(MFT) + 1 + i*sizeof(MFT_ITEM), SEEK_SET);
		fwrite((*vfs) -> mft -> items[i], sizeof(MFT_ITEM), 1, temp_data_file);
		fflush(temp_data_file);
	}

	fseek(temp_data_file, (*vfs) -> boot_record -> bitmap_start_address, SEEK_SET);
	fwrite((*vfs) -> bitmap -> data, sizeof(unsigned char), (*vfs) -> bitmap -> length, temp_data_file);
	fflush(temp_data_file);

	for (i = 0; i < (*vfs) -> mft -> size; i++) {
		actual_size = (*vfs) -> mft -> items[i] -> item_size;
		if (!(*vfs) -> mft -> items[i] -> isDirectory) {
			for (j = 0; j < (*vfs) -> mft -> items[i] -> fragments_created; j++) {
				for (k = 0; k < (*vfs) -> mft -> items[i] -> fragment_count[j]; k++) {
					fseek((*vfs) -> FILE, (*vfs) -> boot_record -> data_start_address + 1 + CLUSTER_SIZE*((*vfs) -> mft -> items[i] -> start_cluster_ID[j] + k), SEEK_SET); 
					fread(buffer, CLUSTER_SIZE, 1, (*vfs) -> FILE);	
					fseek(temp_data_file, (*vfs) -> boot_record -> data_start_address + 1 + CLUSTER_SIZE*((*vfs) -> mft -> items[i] -> start_cluster_ID[j] + k), SEEK_SET); 
					if (actual_size >= CLUSTER_SIZE) {
						actual_size -= CLUSTER_SIZE;
						fwrite(buffer, CLUSTER_SIZE, 1, temp_data_file);
					}
					else {
						fwrite(buffer, actual_size, 1, temp_data_file); 
						break;
					}
					fflush(temp_data_file);
				}
			}
		}
	}
	fflush(temp_data_file);
	fclose(temp_data_file);
	printf("Data copied successfully into temporary data file '%s'\n", TEMP_DATA_FILENAME);
}

void defrag_init_mft_items(VFS **vfs) {
	int i, size = (*vfs) -> mft -> size;

	(*vfs) -> mft -> size = 0;

	mft_item_init(vfs, (*vfs) -> mft -> size, -1, ROOT_NAME, 1, DIRECTORY_SIZE);

	for (i = 1; i < size; i++) {
		if ((*vfs) -> mft -> items[i] -> isDirectory) {
			mft_item_init(vfs, (*vfs) -> mft -> size, (*vfs) -> mft -> items[i] -> parentID, (*vfs) -> mft -> items[i] -> item_name, 1, DIRECTORY_SIZE);
		}
		else {
			int fragment_count = (*vfs) -> mft -> items[i] -> fragments_created;
			int start_id[fragment_count], count[fragment_count]; 

			int j;
			for (j = 0; j < fragment_count; j++) {
				start_id[j] = (*vfs) -> mft -> items[i] -> start_cluster_ID[j];
				count[j] = (*vfs) -> mft -> items[i] -> fragment_count[j];
			}

			mft_item_init(vfs, (*vfs) -> mft -> size, (*vfs) -> mft -> items[i] -> parentID, (*vfs) -> mft -> items[i] -> item_name, 0, (*vfs) -> mft -> items[i] -> item_size);
			defrag_copy_data_back_from_temp_file(vfs, find_mft_item_by_uid((*vfs) -> mft, (*vfs) -> mft -> size - 1), fragment_count, start_id, count);
		}
	}
	printf("MFT Items fragments successfully initialized\n");
}

void defrag_copy_data_back_from_temp_file(VFS **vfs, MFT_ITEM *item, int fragment_count, int start_id[], int count[]) {
	FILE *temp_data_file = fopen(TEMP_DATA_FILENAME, "r");

	int file_part = 1;
	if (item -> item_size != 0) {
		file_part = item -> item_size / CLUSTER_SIZE;
		if (item -> item_size % CLUSTER_SIZE != 0) file_part++;
	}

	/*
	int read_size = CLUSTER_SIZE;
	char buffer[file_part][read_size];
	int position = 0;
	int plus_cluster = 0;
	long actual_size = item -> item_size;
	for (i = 0; i < file_part; i++) {
		fseek(temp_data_file, (*vfs) -> boot_record -> data_start_address + 1 + CLUSTER_SIZE*(start_id[position] + plus_cluster), SEEK_SET);

		plus_cluster++;
		if (actual_size >= CLUSTER_SIZE) {
			actual_size -= CLUSTER_SIZE;
			fread(buffer[i], CLUSTER_SIZE, 1, temp_data_file); 
		}
		else {
			fread(buffer[i], actual_size, 1, temp_data_file); 
			break;
		}
		if (plus_cluster == count[position]) {
			position++;
			plus_cluster = 0;
		}

		//printf("%s", buffer[i]);
	}


	position = 0;
	plus_cluster = 0;
	actual_size = item -> item_size;
	for (i = 0; i < file_part; i++) {
		fseek((*vfs) -> FILE, (*vfs) -> boot_record -> data_start_address + 1 + CLUSTER_SIZE*(start_id[position] + plus_cluster), SEEK_SET);
		plus_cluster++;
		if (actual_size >= CLUSTER_SIZE) {
			actual_size -= CLUSTER_SIZE;
			fwrite(buffer[i], CLUSTER_SIZE, 1, (*vfs) -> FILE); 
		}
		else {
			fwrite(buffer[i], actual_size, 1, (*vfs) -> FILE); 
			break;
		}
		if (plus_cluster == count[position]) {
			position++;
			plus_cluster = 0;
		}
		fflush((*vfs) -> FILE);
	}
	*/

	
	char buffer[CLUSTER_SIZE];

	int i, j, k;
	int actual_size = item -> item_size;
	int plus_cluster = 0;

	for (i = 0; i < fragment_count; i++) {
		for (j = 0; j < count[i]; j++) {
			fseek(temp_data_file, (*vfs) -> boot_record -> data_start_address + 1 + CLUSTER_SIZE*(start_id[i] + j), SEEK_SET);
			for (k = 0; k < CLUSTER_SIZE; k++) {
				buffer[k] = fgetc(temp_data_file);
			}
			fseek((*vfs) -> FILE, (*vfs) -> boot_record -> data_start_address + 1 + CLUSTER_SIZE*(item -> start_cluster_ID[0] + plus_cluster), SEEK_SET); 
			plus_cluster++;
			if (actual_size >= CLUSTER_SIZE) {
				actual_size -= CLUSTER_SIZE;	
				fwrite(buffer, CLUSTER_SIZE, 1, (*vfs) -> FILE); 
			}
			else {
				fwrite(buffer, actual_size, 1, (*vfs) -> FILE); 
				break;
			}
			fflush((*vfs) -> FILE);
		}
	}

	fflush((*vfs) -> FILE);
	
	fclose(temp_data_file);
}

void print_mft(MFT *mft) {
	printf("\nMFT:\n----------------\n");
	printf("Items count: %d\n", mft -> size);
	printf("Size: %lu \n", get_size_of_items(mft));
	printf("\nItems (+ directory, - file):\n");
	int i;
	for (i = 0; i < mft -> size; i++) {
		if (mft -> items[i] -> isDirectory == 1) printf("+");
		else printf("-");
		printf("%s\n", mft -> items[i] -> item_name);
	}
}
