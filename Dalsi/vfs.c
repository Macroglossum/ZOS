#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include "header.h"

#define SIGNATURE "Danisik"
#define DESCRIPTOR "pseudoNTFS"

void vfs_init(VFS **vfs, char *filename, size_t disk_size, int formatting) {

	(*vfs) = calloc(1, sizeof(VFS));

	(*vfs) -> filename = calloc(VFS_FILENAME_LENGTH, sizeof(char));

	if (strlen(filename) > 12) {
		strncpy((*vfs) -> filename, filename, 11);
	}
	else strcpy((*vfs) -> filename, filename);
	
	path_init(vfs);

	FILE *test_if_exists = fopen(filename, "r+");
	if (test_if_exists == NULL || formatting == 1) {
		(*vfs) -> FILE = fopen((*vfs) -> filename, "w+");
		if (formatting == 0) printf("Data file with name %s not found, creating new\n", filename);

		BOOT_RECORD *boot_record;
		boot_record_init(&boot_record, SIGNATURE, DESCRIPTOR, disk_size, CLUSTER_SIZE);
		(*vfs) -> boot_record = boot_record;

		BITMAP *bitmap;
		bitmap_init(&bitmap, (*vfs) -> boot_record -> cluster_count);
		(*vfs) -> bitmap = bitmap;

		mft_init(vfs);

		create_vfs_file(vfs);
	}
	else {
		printf("Data file with name %s found, filling structures\n", filename);
		fread_boot_record(vfs, test_if_exists);
		fread_mft(vfs, test_if_exists);
		fread_bitmap(vfs, test_if_exists);
		fclose(test_if_exists);
		(*vfs) -> FILE = fopen((*vfs) -> filename, "r+");
	}
	
}

void path_init(VFS **vfs) {
	(*vfs) -> actual_path = calloc(1, sizeof(PATH));
	(*vfs) -> actual_path -> path = calloc(PATH_MAX, sizeof(char));
}

void create_vfs_file(VFS **vfs) {
	(*vfs) -> FILE = fopen((*vfs) -> filename, "r+");
	if ((*vfs) -> FILE == NULL) {
		printf("File %s not found\n", (*vfs) -> filename);
		return;
	} 
	else {		
        	fwrite((*vfs) -> boot_record, sizeof(BOOT_RECORD), 1, (*vfs) -> FILE);
		fflush((*vfs) -> FILE);

		fseek((*vfs) -> FILE, (*vfs) -> boot_record -> bitmap_start_address, SEEK_SET);
	        fwrite((*vfs) -> bitmap -> data, sizeof(unsigned char), (*vfs) -> bitmap -> length, (*vfs) -> FILE);
		fflush((*vfs) -> FILE);

		fseek((*vfs) -> FILE, (*vfs) -> boot_record -> data_start_address, SEEK_SET);
	}
}

void set_path_to_root(VFS **vfs) {
	memset((*vfs) -> actual_path -> path, 0, PATH_MAX);
}

void go_to_parent_folder(VFS **vfs) {
	int i, length = array_length_strtok((*vfs) -> actual_path -> path);

	char *tok = strtok((*vfs) -> actual_path -> path, "/");
	char path[PATH_MAX];
	memset(path, 0, PATH_MAX);
	
	for (i = 0; i < length - 1; i++) {
		strcat(path, "/");		
		strcat(path, tok);
		tok = strtok(NULL, "/");	
	}

	set_path_to_root(vfs);
	strcpy((*vfs) -> actual_path -> path, path);
}

void print_vfs(VFS *vfs) {
	print_boot_record(vfs -> boot_record);
	print_mft(vfs -> mft);
	print_bitmap(vfs -> bitmap);
}
