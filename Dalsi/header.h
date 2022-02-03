#define UID_ITEM_FREE 0
#define MFT_FRAGMENTS_COUNT 4
#define VFS_FILENAME_LENGTH 12
#define DISK_SIZE 102400 //10 KB 
#define CLUSTER_SIZE 4096
#define MFT_SIZE_RATIO 0.1
#define DIRECTORY_SIZE 1

#define TEMP_DATA_FILENAME "ntfs_temp.dat"

#define MAX_LENGTH_OF_COMMAND 100
#define SHELL_CHAR "$"
#define ROOT_CHAR "~"
#define SPLIT_ARGS_CHAR " "
#define ROOT_NAME "root"

#define COPY_FILE "cp"
#define MOVE_FILE "mv"
#define REMOVE_FILE "rm"
#define MAKE_DIRECTORY "mkdir"
#define REMOVE_EMPTY_DIRECTORY "rmdir"
#define PRINT_DIRECTORY "ls"
#define PRINT_FILE "cat"
#define MOVE_TO_DIRECTORY "cd"
#define ACTUAL_DIRECTORY "pwd"
#define MFT_ITEM_INFO "info"
#define HD_TO_PSEUDO "incp"
#define PSEUDO_TO_HD "outcp"
#define LOAD_COMMANDS "load"
#define FILE_FORMATTING "format"
#define DEFRAG "defrag"
#define FULL_INFO "finfo"
#define QUIT "quit"
#define HELP "help"


typedef struct the_fragmet_temp FRAGMENT_TEMP;
typedef struct the_bitmap BITMAP;
typedef struct the_mft_item MFT_ITEM;
typedef struct the_mft MFT;
typedef struct the_boot_record BOOT_RECORD;
typedef struct the_path PATH;
typedef struct the_vfs VFS;

struct the_fragment_temp {
	int successful;
	int32_t start_cluster_ID;
	int count;
};

struct the_bitmap {
	int32_t length;
	unsigned char *data;
};

struct the_mft_item {
	int32_t uid;                                       	 //UID polozky, pokud UID = UID_ITEM_FREE, je polozka volna
	int32_t parentID;
	int isDirectory;                                   	 //soubor, nebo adresar (0 soubor, 1 adresar)
	int8_t item_order;                                 	 //poradi v MFT pri vice souborech, jinak 1
	int8_t item_order_total;                           	 //celkovy pocet polozek v MFT, jinak 1
	char item_name[12];                                	 //8+3 + /0 C/C++ ukoncovaci string znak
	long item_size;                                  	//velikost souboru v bytech

	//fragments
	int32_t fragment_start_address[MFT_FRAGMENTS_COUNT];	//start adresa
	int32_t fragment_count[MFT_FRAGMENTS_COUNT];            //pocet clusteru ve fragmentu
	int32_t start_cluster_ID[MFT_FRAGMENTS_COUNT];
	int fragments_created;
};

struct the_mft {
	int32_t size;
	MFT_ITEM **items;
};

struct the_boot_record {
	char signature[9];              //login autora FS
	char volume_descriptor[251];    //popis vygenerovaného FS
	int32_t disk_size;              //celkova velikost VFS
	int32_t cluster_size;           //velikost clusteru
	int32_t cluster_count;          //pocet clusteru
	int32_t mft_start_address;      //adresa pocatku mft
	int32_t bitmap_start_address;   //adresa pocatku bitmapy
	int32_t data_start_address;     //adresa pocatku datovych bloku
	int32_t mft_max_fragment_count; //maximalni pocet fragmentu v jednom zaznamu v mft (pozor, ne souboru)
};

struct the_path {
	char *path;
};

struct the_vfs {
	BOOT_RECORD *boot_record;
	MFT *mft;
	BITMAP *bitmap;
	PATH *actual_path;
	
	char *filename;
	FILE *FILE;	
};


//ntfs.c
void help();

//vfs.c
void vfs_init(VFS **vfs, char *filename, size_t disk_size, int formatting);
void path_init(VFS **vfs);
void create_vfs_file(VFS **vfs);
void set_path_to_root(VFS **vfs);
void go_to_parent_folder(VFS **vfs);
void print_vfs(VFS *vfs);

//boot_record.c
void boot_record_init(BOOT_RECORD **boot_record, char *signature, char *volume_descriptor, int32_t disk_size, int32_t cluster_size);
void fread_boot_record(VFS **vfs, FILE *file);
void print_boot_record(BOOT_RECORD *boot_record);

//mft.c
void mft_init(VFS **vfs);
int mft_item_init(VFS **vfs, int uid, int parentID, char *name, int isDirectory, int item_size);
int mft_fragment_init(VFS **vfs, int cluster_count);
void fread_mft(VFS **vfs, FILE *file);
MFT_ITEM *find_mft_item_by_uid(MFT *mft, int uid);
MFT_ITEM *get_mft_item_from_path(VFS *vfs, char *tok);
void remove_directory(VFS **vfs, int uid);
size_t get_size_of_items(MFT *mft);
int find_folder_id(MFT *mft, char *path);
void print_folder_content(MFT *mft, int parentID);
int is_folder_empty(MFT *mft, int folderID);
void fwrite_mft(VFS **vfs);
void fwrite_mft_item(VFS **vfs, int uid);
int check_if_folder_contains_item(MFT *mft, MFT_ITEM *folder, char *item_name);
void create_file_from_FILE(VFS **vfs, FILE *source, char *source_name, MFT_ITEM *dest);
void remove_given_file(VFS **vfs, MFT_ITEM *file);
void print_file_content(VFS *vfs, MFT_ITEM *item);
void copy_given_file(VFS **vfs, MFT_ITEM *dest_folder, MFT_ITEM *file, char *destination);
void defrag_copy_data_temp_file(VFS **vfs);
void defrag_init_mft_items(VFS **vfs);
void defrag_copy_data_back_from_temp_file(VFS **vfs, MFT_ITEM *item, int fragment_count, int start_id[], int count[]);
void print_mft(MFT *mft);

//bitmap.c
void bitmap_init(BITMAP **bitmap, int32_t cluster_count);
void fread_bitmap(VFS **vfs, FILE *file);
int bitmap_contains_free_cluster(BITMAP *bitmap);
struct the_fragment_temp *find_free_cluster(BITMAP **bitmap, int needed_count);
int used_clusters(BITMAP *bitmap);
void fwrite_bitmap(VFS **vfs);
void defrag_bitmap(VFS **vfs);
void print_bitmap(BITMAP *bitmap);

//commands.c
void copy_file(VFS **vfs, char *tok);
void move_file(VFS **vfs, char *tok);
void remove_file(VFS **vfs, char *tok);
void make_directory(VFS **vfs, char *tok);
void remove_empty_directory(VFS **vfs, char *tok);
void print_directory(VFS *vfs, char *tok);
void print_file(VFS *vfs, char *tok);
void move_to_directory(VFS **vfs, char *tok);
void actual_directory(VFS *vfs);
void mft_item_info(VFS *vfs, char *tok);
void hd_to_pseudo(VFS **vfs, char *tok);
void pseudo_to_hd(VFS **vfs, char *tok);
int load_commands(FILE **file_with_commands, char *tok);
void file_formatting(VFS **vfs, char *tok);
void defrag(VFS **vfs);
void full_info(VFS *vfs);
void commands_help();

//functions.c
int compare_two_string(char *string1, char *string2);
int array_length_strtok(char *path);
int index_of_last_digit(char *size);
int get_multiple(char *multiple, int size);
int directory_exists(char *path);
int my_atoi(const char* snum);
