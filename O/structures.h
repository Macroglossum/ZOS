/*
 * Created by Tomas Linhart on 4. 2. 2021.
 */

#ifndef SP_STRUCTURES_H
#define SP_STRUCTURES_H

#include <stdio.h>
#include <stdint-gcc.h>
#include <stdbool.h>
#include "constants.h"

typedef struct DIR_ITEM {
    int32_t inode;                   // inode odpovídající souboru
    char item_name[12];              //8+3 + /0 C/C++ ukoncovaci string znak
    struct DIR_ITEM *next;           //Spojový seznam všech položek v aktuálním adresáři
} dir_item;


typedef struct DIRECTORY {
    struct DIRECTORY *parent;	/* Reference na adresář o úroveň výš */
    dir_item *current;		 /* Dir_item této složky */
    dir_item *subdir;			/* První prvek spojového seznamu podadresářů */
    dir_item *file;           /* První prvek spojového seznamu souborů */
} directory;

typedef struct INODE {
    int32_t nodeid;                 //ID i-uzlu, pokud ID = ID_ITEM_FREE, je polozka volna
    bool isDirectory;               //soubor, nebo adresar
    int8_t references;              //počet odkazů na i-uzel, používá se pro hardlinky
    int32_t file_size;              //velikost souboru v bytech
    int32_t direct1;                // 1. přímý odkaz na datové bloky
    int32_t direct2;                // 2. přímý odkaz na datové bloky
    int32_t direct3;                // 3. přímý odkaz na datové bloky
    int32_t direct4;                // 4. přímý odkaz na datové bloky
    int32_t direct5;                // 5. přímý odkaz na datové bloky
    int32_t indirect1;              // 1. nepřímý odkaz (odkaz - datové bloky)
    int32_t indirect2;              // 2. nepřímý odkaz (odkaz - odkaz - datové bloky)
} inode;

typedef struct SUPERBLOCK {
    char signature[SIGNATURE_LENGTH];              //login autora FS

    int32_t disk_size;              //celkova velikost VFS
    int32_t cluster_size;           //velikost clusteru
    int32_t cluster_count;          //pocet clusteru
    int32_t inode_count;			// Count of i-nodes
    int32_t bitmap_cluster_count;	// Count of clusters for bitmap
    int32_t inode_cluster_count;	// Count of clusters for i-nodes
    int32_t data_cluster_count;		// Count of clusters for data
    int32_t bitmap_start_address;   // Start address of the bitmap of the data blocks
    int32_t inode_start_address;    // Start address of the i-nodes
    int32_t data_start_address;     // Start address of data blocks
} superblock;

typedef struct vfs {
    superblock *superblock;
    inode *inodes;
    int8_t *data_bitmap;

    bool is_formatted;
    directory *current_dir;
    directory **all_dirs;

    char *name;
    FILE *vfs_file;
} VFS;

#endif //SP_STRUCTURES_H
