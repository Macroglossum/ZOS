
#ifndef FS_FS_H
#define FS_FS_H

// priklad - verze 2020-01
// jedná se o SIMULACI souborového systému
// první i-uzel bude odkaz na hlavní adresář (1. datový blok)
// počet přímých i nepřímých odkazů je v reálném systému větší
// adresář vždy obsahuje dvojici číslo i-uzlu - název souboru
// jde jen o příklad, vlastní datové struktury si můžete upravit

#include <stdbool.h>
#include "constants.h"

typedef struct {
    char signature[10];              //login autora FS
    char volume_descriptor[20];    //popis vygenerovaného FS
    long disk_size;              //celkova velikost VFS
    long cluster_size;           //velikost clusteru
    long cluster_count;          //pocet clusteru
    long bitmapi_start_address;  //adresa pocatku bitmapy i-uzlů
    long bitmap_start_address;   //adresa pocatku bitmapy datových bloků
    long inode_start_address;    //adresa pocatku i-uzlů
    long data_start_address;     //adresa pocatku datovych bloku
} superblock;

typedef struct {
    long nodeid;                 //ID i-uzlu, pokud ID = ID_ITEM_FREE, je polozka volna
    bool isDirectory;               //soubor, nebo adresar
    int8_t references;              //počet odkazů na i-uzel, používá se pro hardlinky
    long file_size;              //velikost souboru v bytech
    long direct1;                // 1. přímý odkaz na datové bloky
    long direct2;                // 2. přímý odkaz na datové bloky
    long direct3;                // 3. přímý odkaz na datové bloky
    long direct4;                // 4. přímý odkaz na datové bloky
    long direct5;                // 5. přímý odkaz na datové bloky
    long indirect1;              // 1. nepřímý odkaz (odkaz - datové bloky)
    long indirect2;              // 2. nepřímý odkaz (odkaz - odkaz - datové bloky)
} inode;


typedef struct {
    long inode;                   // inode odpovídající souboru
    char item_name[12];              //8+3 + /0 C/C++ ukoncovaci string znak
} directory_item;

extern char *fs;
extern superblock *sb;
extern inode *inodes;
extern int8_t* bitmap;
extern char *data_blocks;
extern int bitmap_size;
extern int inodes_count;
extern int items_per_block;
extern char* fs_file_name;
extern char current_path[BUFFER_SIZE];
extern int current_inode;
extern int root_inode;
extern int block_indexes_per_block;

// funkce pro predvyplneni struktury sb

//void fill_default_sb(struct superblock *sb) {
//    // add disk_size param
//    strcpy(sb->signature, "== ext ==");
//    strcpy(sb->volume_descriptor, "popis bla bla bla");
//    sb->disk_size = 800; // 800B
//    sb->cluster_size = 32; // takže max. počet "souborových" položek na cluster jsou dvě, protože sizeof(directory_item) = 16B
//    sb->cluster_count = 10;
//    sb->bitmapi_start_address = sizeof(struct superblock); // konec sb
//    sb->bitmap_start_address = sb->bitmapi_start_address + 10; // 80 bitů bitmapa
//    sb->inode_start_address = sb->bitmap_start_address + 10;
//    sb->data_start_address = sb->inode_start_address + 10 * sizeof(struct pseudo_inode);
//}

int init_fs(char *fs_name);
char *read_file(char *file_name);
int write_file(char *file, char *name);
int write_file_b(char *file, char *name);
int load_fs(char *fs_name);
void set_variables(char *fs_file);

void init_inodes();
void init_root();
int32_t get_inodes_byte_count(int32_t size);
int32_t get_data_blocks_count(int32_t size, int32_t inodes_byte_count);
int32_t get_data_blocks_byte_count(int32_t data_blocks_count);
int32_t get_bitmap_bytes_count(int32_t data_blocks_count);
int32_t get_final_fs_bytes_count(int32_t bitmap_bytes, int32_t inodes_bytes, int32_t blocks_bytes);

//Utils
char *block_by_id(int block_index);
int get_free_inode();
int get_free_block();
directory_item *get_free_dir_item(char *data_block);
char *format_path(char *path);
int get_inode_block(inode *node, int index);
int add_block_to_inode(inode *node, int block_id);
void copy_block(char *block_source, char *block_dest);
directory_item *item_by_name(char *data_block, char *name);
inode *inode_by_id(int inode_id);
int inode_by_path(char *path);
int inode_parent(inode *node);
void free_block(int block_index);
void free_inode(int inode_id);

int format_fs(int32_t size);
int create_dir(char *dir_name);
int remove_dir(char *path);
int print_dir(char *path);
int copy_file(char *path, char *dest);
int move_file(char *path, char *dest);
int remove_file(char *path);
int change_dir(char *path);
int in_copy_file(char *file_name, char *path);
int out_copy_file(char *file_name, char *path);
int print_file(char *path);
#endif