#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"


void bitmap_init(BITMAP **bitmap, int32_t cluster_count) {
	(*bitmap) = calloc(1, sizeof(BITMAP));
	(*bitmap) -> length = cluster_count;
	(*bitmap) -> data = calloc((*bitmap) -> length, sizeof(unsigned char));
	memset((*bitmap) -> data, 0, (*bitmap) -> length);
}

void fread_bitmap(VFS **vfs, FILE *file) {
	(*vfs) -> bitmap = calloc(1, sizeof(BITMAP));
	(*vfs) -> bitmap -> length = (*vfs) -> boot_record -> cluster_count;
	(*vfs) -> bitmap -> data = calloc((*vfs) -> boot_record -> cluster_count, sizeof(unsigned char));
	fseek(file, (*vfs) -> boot_record -> bitmap_start_address, SEEK_SET);	
	fread((*vfs) -> bitmap -> data, sizeof(char unsigned), (*vfs) -> boot_record -> cluster_count, file);	
}

int bitmap_contains_free_cluster(BITMAP *bitmap) {
	int i;
	
	for (i = 0; i < bitmap -> length; i++) {
		if (bitmap -> data[i] == 0) {
			return 0;
		}
	}
	return 1;
}

struct the_fragment_temp *find_free_cluster(BITMAP **bitmap, int needed_count) {
	struct the_fragment_temp *temp = calloc(1, sizeof(struct the_fragment_temp));
	temp -> start_cluster_ID = -1;
	temp -> count = 0;
	temp -> successful = 0;	
	int i;
	int count = 0;
	for (i = 0; i < (*bitmap) -> length; i++) {
		if ((*bitmap) -> data[i] == 0) {
			if (count == 0) temp -> start_cluster_ID = i;
			(*bitmap) -> data[i] = 1;			
			count++;		
			temp -> count = count;			

			if (count == needed_count) {
				temp -> successful = 1;
				return temp;
			}
		}
		else {
			if (count > 0) {
				temp -> successful = 0;
				return temp;
			}
		}
	}
	return temp;
}

void fwrite_bitmap(VFS **vfs) {
	fseek((*vfs) -> FILE, (*vfs) -> boot_record -> bitmap_start_address, SEEK_SET);
	fwrite((*vfs) -> bitmap -> data, sizeof(unsigned char), (*vfs) -> bitmap -> length, (*vfs) -> FILE);
	fflush((*vfs) -> FILE);
}

int used_clusters(BITMAP *bitmap) {
	int clusters_used = 0;
	int i;
	for (i = 0; i < bitmap -> length; i++) {
		if (bitmap -> data[i] == 1) clusters_used++;
	}

	return clusters_used;
}

void defrag_bitmap(VFS **vfs) {
	int i;
	for (i = 0; i < (*vfs) -> bitmap -> length; i++) {
		(*vfs) -> bitmap -> data[i] = 0;
	}

	fwrite_bitmap(vfs);
	printf("Bitmap cleared\n");
}

void print_bitmap(BITMAP *bitmap) {
	int clusters_used = used_clusters(bitmap);
	printf("\nBitmap:\n----------------\n");
	printf("Length: %d\n", bitmap -> length);
	printf("Clusters ((%dx) 0 - not used; (%dx) 1 - used):", bitmap -> length - clusters_used, clusters_used);
	
	int i;
	for (i = 0; i < bitmap -> length; i++) {
		if (i % 8 == 0) printf("\n");		
		printf("%d", bitmap -> data[i]);
	}
	printf("\n");
}
