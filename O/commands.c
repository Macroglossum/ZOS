/*
 * Created by Tomas on 4. 2. 2021.
 */

#include "commands.h"
#include "utils.h"
#include "directory.h"
#include "vfs.h"
#include "macros.h"

/*
 * Processess command passed in the input parameter
 */
int process_command_line(VFS **vfs, char *input) {
    char *command = strtok(input, " ");

    if (streq(command, INCP_COMMAND)) {
        if (verify_format(vfs)) {
            char *filename_src = strtok(NULL, " ");
            char *filename_dest = strtok(NULL, " ");

            if (strempty(filename_src)) {
                printf(SRC_NOT_DEFINED_MSG);
                return false;
            }

            if (strempty(filename_dest)) {
                printf(DEST_NOT_DEFINED_MSG);
                return false;
            }

            incp(vfs, filename_src, filename_dest);
        }

        return false;
    } else if (streq(command, OUTCP_COMMAND)) {
        if (verify_format(vfs)) {
            char *filename_src = strtok(NULL, " ");
            char *filename_dest = strtok(NULL, " ");

            if (strempty(filename_src)) {
                printf(SRC_NOT_DEFINED_MSG);
                return false;
            }

            if (strempty(filename_dest)) {
                printf(DEST_NOT_DEFINED_MSG);
                return false;
            }

            outcp(vfs, filename_src, filename_dest);
        }

        return false;
    } else if (streq(command, CP_COMMAND)) {
        if (verify_format(vfs)) {
            char *filename_src = strtok(NULL, " ");
            char *filename_dest = strtok(NULL, " ");

            if (strempty(filename_src)) {
                printf(SRC_NOT_DEFINED_MSG);
                return false;
            }

            if (strempty(filename_dest)) {
                printf(DEST_NOT_DEFINED_MSG);
                return false;
            }

            cp(vfs, filename_src, filename_dest);
        }

        return false;
    } else if (streq(command, MV_COMMAND)) {
        if (verify_format(vfs)) {
            char *filename_src = strtok(NULL, " ");
            char *filename_dest = strtok(NULL, " ");

            if (strempty(filename_src)) {
                printf(SRC_NOT_DEFINED_MSG);
                return false;
            }

            if (strempty(filename_dest)) {
                printf(DEST_NOT_DEFINED_MSG);
                return false;
            }

            mv(vfs, filename_src, filename_dest);
        }

        return false;
    } else if (streq(command, MKDIR_COMMAND)) {
        if (verify_format(vfs)) {
            char *dir_name = strtok(NULL, " ");

            if (strempty(dir_name)) {
                printf(SRC_NOT_DEFINED_MSG);
                return false;
            }

            vfs_mkdir(vfs, dir_name);
        }

        return false;
    } else if (streq(command, RM_COMMAND)) {
        if (verify_format(vfs)) {
            char *filename = strtok(NULL, " ");

            if (strempty(filename)) {
                printf(SRC_NOT_DEFINED_MSG);
                return false;
            }

            rm(vfs, filename);
        }

        return false;
    } else if (streq(command, RMDIR_COMMAND)) {
        if (verify_format(vfs)) {
            char *dir_name = strtok(NULL, " ");

            if (strempty(dir_name)) {
                printf(SRC_NOT_DEFINED_MSG);
                return false;
            }

            vfs_rmdir(vfs, dir_name);
        }

        return false;
    } else if (streq(command, CD_COMMAND)) {
        if (verify_format(vfs)) {
            char *dir_name = strtok(NULL, " ");

            if (strempty(dir_name)) {
                printf(SRC_NOT_DEFINED_MSG);
                return false;
            }

            cd(vfs, dir_name);
        }

        return false;
    } else if (streq(command, LS_COMMAND)) {
        if (verify_format(vfs)) {
            char *dir_name = strtok(NULL, " ");

            if (strempty(dir_name)) {
                dir_name = ".";
            }

            ls(vfs, dir_name);
        }

        return false;
    } else if (streq(command, CAT_COMMAND)) {
        if (verify_format(vfs)) {
            char *filename = strtok(NULL, " ");

            if (strempty(filename)) {
                printf(SRC_NOT_DEFINED_MSG);
                return false;
            }

            cat(vfs, filename);
        }

        return false;
    } else if (streq(command, INFO_COMMAND)) {
        if (verify_format(vfs)) {
            char *filename = strtok(NULL, " ");

            if (strempty(filename)) {
                printf(SRC_NOT_DEFINED_MSG);
                return false;
            }

            info(vfs, filename);
        }

        return false;
    } else if (streq(command, PWD_COMMAND)) {
        if (verify_format(vfs)) {
            pwd(vfs);
        }

        return false;
    } else if (streq(command, DEBUG_COMMAND)) {
        if (verify_format(vfs)) {
            debug(vfs);
        }

        return false;
    } else if (streq(command, CHECK_COMMAND)) {
        if (verify_format(vfs)) {
            check(vfs);
        }

        return false;

    } else if (streq(command, LOAD_COMMAND)) {
        char *filename = strtok(NULL, " ");

        load(vfs, filename);

        return false;
    } else if (streq(command, FORMAT_COMMAND)) {
        char *size_char = strtok(NULL, " ");
        int32_t filesystem_size = get_size_from_string(size_char);

        e_return_value_on_error(filesystem_size, FORMAT_ERROR_MSG, false);
        e_return_value_on_condition(filesystem_size < MIN_FS, FORMAT_ERROR_SIZE_MSG, false);

        format(vfs, filesystem_size);

        return false;

    } else if (streq(command, SIZE_COMMAND)) {
        char *id_char = strtok(NULL, " ");
        int32_t inode_id = convert_int(id_char);
        e_return_value_on_condition(inode_id == INT32_MIN, SIZE_WRONG_INODE_MSG, false);

        char *size_char = strtok(NULL, " ");
        int32_t filesystem_size = get_size_from_string(size_char);

        e_return_value_on_error(filesystem_size, FILESIZE_CONVERT_ERROR, false);

        size(vfs, inode_id, filesystem_size);

        return false;
    } else if (streq(command, EXIT_COMMAND)) {
        printf("/--------------------\\\n");
        printf("|   END OF PROGRAM   |\n");
        printf("\\--------------------/\n\n");
        return true;
    } else if (streq(command, HELP_COMMAND)) {
        help();
        return false;
    } else {
        printf(UNKNOWN_COMMAND_MSG, command);
        return false;
    }
}

/*
 * Shows how to use the program
 */
void help() {
    printf("/----------\\\n");
    printf("|   HELP   |\n");
    printf("\\----------/\n");
    printf("\n");
    printf("Spuštění programu: \n\n");
    printf("./vfs [jméno_filesystému]\n\n");
    printf("Dostupné příkazy: \n\n");
    printf("cp s1 s2  --  Zkopíruje soubor na cestě s1 na cestu s2\n");
    printf("mv s1 s2  --  Přesune soubor na cestě s1 na cestu s2\n");
    printf("rm s1  --  Odstraní soubor s1\n");
    printf("mkdir a1  --  Vytvoří adresář a1\n");
    printf("rmdir a1  --  Odstraní adresář a1\n");
    printf("ls a1  --  Vypíše obsah adresáře a1\n");
    printf("cat s1  --  Vypíše obsah souboru s1\n");
    printf("cd a1  --  Změní aktuální složku na adresář na adrese a1\n");
    printf("pwd  --  Vypíše cestu k aktuální složce\n");
    printf("info a1/s1  --  Vypíše informace o daném souboru / složce\n");
    printf("incp s1 s2  --  Přesune soubor z reálného souborového systému do virtuálního\n");
    printf("outcp s1 s2  --  Přesune soubor z virtuálního souborového systému do reálného\n");
    printf("load s1  --  Přečte příkazy řádek po řádku ze souboru s1 z reálného souborového systému\n");
    printf("format 600M  --  Naformátuje virtuální souborový systém (VFS)\n");
    printf("check  --  Provede kontrolu konzistence souborového systému\n");
    printf("size inode_id 600M  --  Změní velikost i-nodu s daným ID na velikost specifikovanou parametrem\n");
}

/*
 * Shows debug information
 */
void debug(VFS **vfs) {
    printf("Signature : %s\n"
           "Disk size: %d\n"
           "Cluster size: %d\n"
           "Cluster count: %d\n"
           "Max Inode Count: %d\n"
           "Bitmap cluster count: %d\n"
           "Inode cluster count: %d\n"
           "Data cluster count: %d\n"
           "Bitmap start address: %d\n"
           "Inode start address: %d\n"
           "Data start address: %d\n",
           (*vfs)->superblock->signature,
           (*vfs)->superblock->disk_size,
           (*vfs)->superblock->cluster_size,
           (*vfs)->superblock->cluster_count,
           (*vfs)->superblock->inode_count,
           (*vfs)->superblock->bitmap_cluster_count,
           (*vfs)->superblock->inode_cluster_count,
           (*vfs)->superblock->data_cluster_count,
           (*vfs)->superblock->bitmap_start_address,
           (*vfs)->superblock->inode_start_address,
           (*vfs)->superblock->data_start_address);

    printf("\nVytvořené Inode :\n");
    for (unsigned long i = 0 ; i < (*vfs)->superblock->inode_count; i++){
        if ((*vfs)->inodes[i].nodeid == ID_ITEM_FREE) {
            continue;
        }

        printf("%d ",(*vfs)->inodes[i].nodeid);
    }

    printf("\nData bitmapa:\n");
    for (int i = 0 ; i < (*vfs)->superblock->data_cluster_count; i++){
        printf("%d", (*vfs)->data_bitmap[i]);
    }
    printf("\n");
}

/*
 * Formats VFS file
 */
void format(VFS **vfs, int32_t filesystem_size) {
    FILE *temp_vfs_file = fopen((*vfs)->name, "wb+");
    e_return_on_null(temp_vfs_file, OPEN_FILE_ERR_MSG);

    (*vfs)->vfs_file = temp_vfs_file;
    (*vfs)->is_formatted = true;

    (*vfs)->superblock = superblock_init(filesystem_size);

    /* Prepare bitmap, i-nodes and pointers to directories */
    (*vfs)->data_bitmap = calloc(1, (*vfs)->superblock->cluster_count);
    e_return_on_null((*vfs)->data_bitmap, MEMORY_ERROR_MSG);

    (*vfs)->inodes = calloc((*vfs)->superblock->inode_count, sizeof(inode));
    e_return_on_null((*vfs)->inodes, MEMORY_ERROR_MSG);

    (*vfs)->all_dirs = calloc((*vfs)->superblock->inode_count, sizeof(directory *)); /*Kdyby všechny položky ve FS byly složk */
    e_return_on_null((*vfs)->all_dirs, MEMORY_ERROR_MSG);

    /* Create root */
    directory *root = calloc(1, sizeof(directory));
    e_return_on_null(root, MEMORY_ERROR_MSG);

    root->current = new_directory_item(0, "/");
    root->parent = root;
    root->subdir = NULL;
    root->file = NULL;

    (*vfs)->current_dir = root;	/* Set root as working directory */
    (*vfs)->all_dirs[0] = root;

    /* Clear bitmap */
    for (int i = 0; i < (*vfs)->superblock->data_cluster_count; i++) {
        (*vfs)->data_bitmap[i] = 0;
    }


    /* Set all i-nodes as free */
    for (int i = 0; i < (*vfs)->superblock->inode_count; i++) {
        (*vfs)->inodes[i].nodeid = ID_ITEM_FREE;
        (*vfs)->inodes[i].isDirectory = 0;
        (*vfs)->inodes[i].references = 0;
        (*vfs)->inodes[i].file_size = 0;
        (*vfs)->inodes[i].direct1 = ID_ITEM_FREE;
        (*vfs)->inodes[i].direct2 = ID_ITEM_FREE;
        (*vfs)->inodes[i].direct3 = ID_ITEM_FREE;
        (*vfs)->inodes[i].direct4 = ID_ITEM_FREE;
        (*vfs)->inodes[i].direct5 = ID_ITEM_FREE;
        (*vfs)->inodes[i].indirect1 = ID_ITEM_FREE;
        (*vfs)->inodes[i].indirect2 = ID_ITEM_FREE;
    }

    /*Set first cluster of data bitmap - root directory */
    (*vfs)->data_bitmap[0] = 1;

    /*Set first inode - root directory */
    (*vfs)->inodes[0].nodeid = 0;
    (*vfs)->inodes[0].isDirectory = 1;
    (*vfs)->inodes[0].references = 1;
    (*vfs)->inodes[0].direct1 = 0;

    /* Fill the file with zeros */
    char buffer[CLUSTER_SIZE];
    memset(buffer, 0, CLUSTER_SIZE);
    for (int i = 0; i < (*vfs)->superblock->cluster_count; i++) {
        write_vfs(vfs, buffer, sizeof(buffer), 1);
    }


    /* Store the superblock */
    rewind_vfs(vfs);
    write_vfs(vfs, &((*vfs)->superblock->signature), sizeof(char) * SIGNATURE_LENGTH, 1);
    int32_write_vfs(vfs, &((*vfs)->superblock->disk_size));
    int32_write_vfs(vfs, &((*vfs)->superblock->cluster_size));
    int32_write_vfs(vfs, &((*vfs)->superblock->cluster_count));
    int32_write_vfs(vfs, &((*vfs)->superblock->inode_count));
    int32_write_vfs(vfs, &((*vfs)->superblock->bitmap_cluster_count));
    int32_write_vfs(vfs, &((*vfs)->superblock->inode_cluster_count));
    int32_write_vfs(vfs, &((*vfs)->superblock->data_cluster_count));
    int32_write_vfs(vfs, &((*vfs)->superblock->bitmap_start_address));
    int32_write_vfs(vfs, &((*vfs)->superblock->inode_start_address));
    int32_write_vfs(vfs, &((*vfs)->superblock->data_start_address));

    int8_t o = 1;

    /* Store bitmap - data block 0 (root) */
    seek_set(vfs, (*vfs)->superblock->bitmap_start_address);
    int8_write_vfs(vfs, &o);

    /* Store i-nodes */
    for (int i = 0; i < (*vfs)->superblock->inode_count; i++) {
        write_inode_to_vfs(vfs, i);
    }

    (*vfs)->is_formatted = 1;
    printf(FORMAT_SUCCESS_MSG);
}

/*
 * Copies file from real filesystem into VFS.
 */
void incp(VFS** vfs, char *filepath_src, char *filepath_dest) {
    int32_t *blocks, inode_id;
    int i, block_count, real_block_count, tmp, last_block_index;
    char *name;
    directory *dir;
    dir_item **new_dir_item;


    /* Find destination directory */
    e_return_on_condition(parse_path(vfs, filepath_dest, &name, &dir), PATH_NOT_FOUND_MSG);

    e_return_on_condition(strlen(name) >= FILENAME_LENGTH, FILENAME_TOO_LONG_MSG);

    e_return_on_condition(vfs_exists(dir, name) == true, FILE_EXISTS_MSG);

    if (strlen(name) == 0) {
        name = strrchr(filepath_src, '/');

        if (name == NULL) {
            name = filepath_src;
        } else {
            name++; /* Skip the slash */
        }
    }

    FILE *src_file = fopen(filepath_src, "rb");
    e_return_on_null(src_file, FILE_NOT_FOUND_MSG);

    /* Get size of the copied file */
    int fd = fileno(src_file);
    struct stat buf;
    fstat(fd, &buf);
    int32_t file_size = buf.st_size;

    block_count = file_size / CLUSTER_SIZE;
    /* Need one more block to store partial data */
    if (file_size % CLUSTER_SIZE != 0) block_count++;

    real_block_count = get_block_count_with_indirect(block_count);

    blocks = find_free_data_blocks(vfs, real_block_count);
    e_return_on_null(blocks, NOT_ENOUGH_BLOCKS_MSG);

    /* Get ID of a free i-node */
    inode_id = find_free_inode(vfs);


    if (inode_id == ERROR_CODE) {
        printf(NO_FREE_INODE_MSG);
        fclose(src_file);
        return;
    }

    new_dir_item = &(dir->file);
    while (*new_dir_item != NULL) {        /* Loop to the end of the list */
        new_dir_item = &((*new_dir_item)->next);
    }

    *new_dir_item = new_directory_item(inode_id, name);

    /* Initialize i-node */
    last_block_index = initialize_inode(vfs, inode_id, file_size, block_count, blocks);

    update_bitmap_in_file(vfs, *new_dir_item, 1, blocks, block_count);
    write_inode_to_vfs(vfs, inode_id);
    update_directory_in_file(vfs, dir, *new_dir_item, true); /* TODO */
    update_sizes_in_file(vfs, dir, file_size);


    /* Initialize buffer */
    char buffer[CLUSTER_SIZE];
    memset(buffer, 0, CLUSTER_SIZE);


    /* Read block, seek in VFS and write into VFS */
    for (i = 0; i < block_count - 1; i++) {
        fread(buffer, sizeof(buffer), 1, src_file);
        seek_data_cluster(vfs, blocks[i]);
        write_vfs(vfs, buffer, sizeof(buffer), 1);
    }

    /* Read last block */
    tmp = get_last_block_size(file_size % CLUSTER_SIZE);

    /* TODO CHANGE EVERYWHERE TO PART BUFFER */
    char part_buffer[tmp];

    fread(buffer, sizeof(part_buffer), 1, src_file);
    seek_data_cluster(vfs, blocks[last_block_index]);
    write_vfs(vfs, part_buffer, sizeof(part_buffer), 1);

    flush_vfs(vfs);

    /* Cleanup */
    fclose(src_file);
    free(blocks);

    printf(OK_MSG);
}

/*
 * Kopíruje soubor ven z virtuálního souborového systému
 */
void outcp(VFS **vfs, char *filepath_src, char *filepath_dest) {
    int i, block_count, rest, last_block_size;
    int32_t *blocks;
    char *name;
    char buffer[CLUSTER_SIZE];
    directory *dir;
    dir_item *item;
    FILE *f;

    /* Parse source path */
    e_return_on_error(parse_path(vfs, filepath_src, &name, &dir), FILE_NOT_FOUND_MSG);

    /* Find item */
    item = find_item(dir->file, name);
    e_return_on_null(item, FILE_NOT_FOUND_MSG);

    /* Open output file */
    f = fopen(filepath_dest, "wb");
    e_return_on_null(f, PATH_NOT_FOUND_MSG);

    blocks = get_data_blocks(vfs, item->inode, &block_count, &rest);

    /* Copy data blocks */
    for (i = 0; i < block_count - 1; i++) {
        seek_data_cluster(vfs, blocks[i]);
        read_vfs(vfs, buffer, sizeof(buffer), 1);
        fwrite(buffer, sizeof(buffer), 1, f);
        fflush(f);
    }

    /* Copy last block */
    last_block_size = get_last_block_size(rest);
    char part_buffer[last_block_size];

    seek_data_cluster(vfs, blocks[block_count - 1]);
    read_vfs(vfs, part_buffer, sizeof(part_buffer), 1);
    fwrite(part_buffer, sizeof(part_buffer), 1, f);

    fflush(f);
    flush_vfs(vfs);
    fclose(f);
    free(blocks);

    printf(OK_MSG);
}

/*
 * Creates new directory specified in the path.
 */
void vfs_mkdir(VFS** vfs, char *dir_name) {
    int32_t inode_id, *data_block;
    directory *dir;
    char *name;

    /* Parse path */
    e_return_on_error(parse_path(vfs, dir_name, &name, &dir), PATH_NOT_FOUND_MSG);

    /* Test if destination folder doesn't contain file/directory with the same name */
    e_return_on_condition(vfs_exists(dir, name), FILE_EXISTS_MSG);


    /* Get free inode */
    inode_id = find_free_inode(vfs);
    e_return_on_error(inode_id, NOT_ENOUGH_BLOCKS_MSG);

    /* Get free data block */
    data_block = find_free_data_blocks(vfs, 1);
    e_return_on_null(data_block, NOT_ENOUGH_BLOCKS_MSG);

    /* Create directory */
    directory *newdir = calloc(1, sizeof(directory));
    newdir->parent = dir;
    newdir->current = new_directory_item(inode_id, name);
    newdir->file = NULL;
    newdir->subdir = NULL;

    /* Save new directory to list of directories */
    (*vfs)->all_dirs[inode_id] = newdir;

    /* Allocate data block for directory in bitmap */
    (*vfs)->data_bitmap[data_block[0]] = 1;

    /* Initialize i-node of a new directory */
    (*vfs)->inodes[inode_id].nodeid = inode_id;
    (*vfs)->inodes[inode_id].isDirectory = 1;
    (*vfs)->inodes[inode_id].references = 1;
    (*vfs)->inodes[inode_id].file_size = 0;
    (*vfs)->inodes[inode_id].direct1 = data_block[0];


    /* Loop to the end of current directory */
    dir_item** temp = &(dir->subdir);
    while (*temp != NULL) {
        temp = &((*temp)->next);
    }

    /* Add directory to the end of the list */
    *temp = (*vfs)->all_dirs[inode_id]->current;

    /* Update parent directory */
    e_return_on_error(update_directory_in_file(vfs, dir, newdir->current, true), NOT_ENOUGH_BLOCKS_MSG);

    /* Update inodes and bitmap */
    write_inode_to_vfs(vfs, inode_id);
    update_bitmap_in_file(vfs, newdir->current, 1, data_block, 1);
    free(data_block);

    printf(OK_MSG);
}

/*
 * Changes current directory in VFS
 */
void cd(VFS** vfs, char *path) {
    /* Find directory */
    directory *dir = find_directory(vfs, path);
    e_return_on_null(dir, PATH_NOT_FOUND_MSG);

    /* Set as current dir for the vfs */
    (*vfs)->current_dir = dir;
    printf(OK_MSG);
}

/*
 * Lists out contents of directory in the spcified path
 */
void ls(VFS** vfs, char *path) {
    directory *dir;
    dir_item *item;

    /* Find directory in command */
    dir = find_directory(vfs, path);
    e_return_on_null(dir, PATH_NOT_FOUND_MSG);

    /* Print out all subdirs */
    item = dir->subdir;
    while (item != NULL) {
        printf("+%s\n", item->item_name);
        item = item->next;
    }

    /* Print out all files */
    item = dir->file;
    while (item != NULL) {
        printf("-%s\n", item->item_name);
        item = item->next;
    }
}

/* TODO EDIT */
void cat(VFS** vfs, char *file) {
    directory *dir;
    dir_item *item;
    char *name;
    int i, last_block_size, block_count, rest;
    char buffer[CLUSTER_SIZE];

    /* Parse path */
    e_return_on_error(parse_path(vfs, file, &name, &dir), FILE_NOT_FOUND_MSG);

    item = find_item(dir->file, name);
    e_return_on_null(item, FILE_NOT_FOUND_MSG);

    /* Print out whole blocks */
    int32_t *blocks = get_data_blocks(vfs, item->inode, &block_count, &rest);
    memset(buffer, 0, CLUSTER_SIZE);
    for (i = 0; i < block_count - 1; i++) {
        seek_data_cluster(vfs, blocks[i]);

        read_vfs(vfs, buffer, CLUSTER_SIZE, 1);
        printf("%s", buffer);
        memset(buffer, 0, CLUSTER_SIZE);
    }

    /* Print out last block */
    last_block_size = get_last_block_size(rest);
    seek_data_cluster(vfs, blocks[block_count - 1]);
    read_vfs(vfs, buffer, last_block_size, 1);
    printf("%s", buffer);

    free(blocks);
    printf("\n");
}

/*
 * Print out path of the current directory
 */
void pwd(VFS** vfs) {
    directory *temp_dir = (*vfs)->current_dir;
    char buffer[256];

    memset(buffer, 0, 256);
    while(temp_dir->current->inode != (*vfs)->all_dirs[0]->current->inode) { /* While not root */
        prepend(buffer, temp_dir->current->item_name);
        prepend(buffer, "/");

        temp_dir = temp_dir->parent;
    }

    if (strempty(buffer)) { /* If pwd in root */
        printf("/");
    }

    printf("%s\n", buffer);
}

/*
 * Print info about file specified in path
 */
void info(VFS** vfs, char *path) {
    directory *dir;
    char *name;
    dir_item *item;

    if (streq(path, ".")) {
        print_dir_item_info(vfs, (*vfs)->current_dir->current);
        return;
    }

    /* Parse path */
    e_return_on_error(parse_path(vfs, path, &name, &dir), FILE_NOT_FOUND_MSG)

    /* Checks if directory is root */
    if (dir == (*vfs)->all_dirs[0] && strlen(name) == 0) {
        print_dir_item_info(vfs, (*vfs)->all_dirs[0]->current);
        return;
    }

    /* Find file */
    item = find_item(dir->file, name);
    if (item != NULL) {
        print_dir_item_info(vfs, item);
        return;
    }

    /* Find directory */
    item = find_item(dir->subdir, name);
    if (item != NULL) {
        print_dir_item_info(vfs, item);
        return;
    }

    printf(FILE_NOT_FOUND_MSG);
}

/*
 * Removes file from the VFS
 */
void rm(VFS **vfs, char *file) {
    int i, block_count, rest;
    int32_t *blocks;
    char *name;
    directory *dir;
    dir_item *item, **temp;
    bool dir_item_found = false;

    e_return_on_error(parse_path(vfs, file, &name, &dir), FILE_NOT_FOUND_MSG);

    /* Remove file from the list */
    temp = &(dir->file);
    item = dir->file;
    while (item != NULL) {
        if (streq(name, item->item_name)) {
            /* Write next file to the address of this file */
            (*temp) = item->next;
            dir_item_found = true;
            break;
        }

        /* Skip to next file */
        temp = &(item->next);
        item = item->next;
    }

    e_return_on_condition(dir_item_found == false, FILE_NOT_FOUND_MSG);

    /* Get blocks for the deleted file */
    blocks = get_data_blocks(vfs, item->inode, &block_count, &rest);

    /* Replace blocks with zeros */
    char buffer[CLUSTER_SIZE];
    memset(buffer, 0, CLUSTER_SIZE);

    /* Erase all data blocks - We can erase even the partial one - Other files don't use this block */
    for (i = 0; i < block_count; i++) {
        seek_data_cluster(vfs, blocks[i]);
        write_vfs(vfs, buffer, sizeof(buffer), 1);
    }

    /* Erase indirect references */
    if ((*vfs)->inodes[item->inode].indirect1 != ID_ITEM_FREE) {
        seek_data_cluster(vfs, (*vfs)->inodes[item->inode].indirect1);
        write_vfs(vfs, buffer, sizeof(buffer), 1);

        if ((*vfs)->inodes[item->inode].indirect2 != ID_ITEM_FREE) {
            seek_data_cluster(vfs, (*vfs)->inodes[item->inode].indirect2);
            write_vfs(vfs, buffer, sizeof(buffer), 1);
        }
    }

    flush_vfs(vfs);

    /* Erase blocks in bitmap */
    update_bitmap_in_file(vfs, item, 0, blocks, block_count);

    /* Update size of directories */
    update_sizes_in_file(vfs, dir, -((*vfs)->inodes[item->inode].file_size));

    /* Remove the directory from VFS file */
    update_directory_in_file(vfs, dir, item, false);

    /* Erase inode data */
    (*vfs)->inodes[item->inode].nodeid = ID_ITEM_FREE;
    (*vfs)->inodes[item->inode].isDirectory = 0;
    (*vfs)->inodes[item->inode].references = 0;
    (*vfs)->inodes[item->inode].file_size = 0;
    (*vfs)->inodes[item->inode].direct1 = ID_ITEM_FREE;
    (*vfs)->inodes[item->inode].direct2 = ID_ITEM_FREE;
    (*vfs)->inodes[item->inode].direct3 = ID_ITEM_FREE;
    (*vfs)->inodes[item->inode].direct4 = ID_ITEM_FREE;
    (*vfs)->inodes[item->inode].direct5 = ID_ITEM_FREE;
    (*vfs)->inodes[item->inode].indirect1 = ID_ITEM_FREE;
    (*vfs)->inodes[item->inode].indirect2 = ID_ITEM_FREE;

    /* Write inode data to file */
    write_inode_to_vfs(vfs, item->inode);

    free(item);
    free(blocks);

    printf(OK_MSG);
}

/*
 * Physically remove directory from the VFS
 */
void remove_dir(VFS **vfs, directory *parent, dir_item **current_item_address, dir_item *current_item) {
    directory *current_dir_item = (*vfs)->all_dirs[current_item->inode];
    e_return_on_condition((current_dir_item->file != NULL) || (current_dir_item->subdir != NULL), DIR_NOT_EMPTY_MSG);

    (*current_item_address) = current_item->next;

    if ((*vfs)->current_dir == current_dir_item) {	/* Cannot remove the directory that I'm working in.. I have to move one folder higher */
        (*vfs)->current_dir = current_dir_item->parent;
    }

    /* Remove directory from bitmap */
    update_bitmap_in_file(vfs, current_item, 0, NULL, 0);

    /* Free Inode */
    (*vfs)->inodes[current_item->inode].nodeid = ID_ITEM_FREE;
    (*vfs)->inodes[current_item->inode].isDirectory = 0;
    (*vfs)->inodes[current_item->inode].references = 0;
    (*vfs)->inodes[current_item->inode].file_size = 0;
    (*vfs)->inodes[current_item->inode].direct1 = ID_ITEM_FREE;
    (*vfs)->inodes[current_item->inode].direct2 = ID_ITEM_FREE;
    (*vfs)->inodes[current_item->inode].direct3 = ID_ITEM_FREE;
    (*vfs)->inodes[current_item->inode].direct4 = ID_ITEM_FREE;
    (*vfs)->inodes[current_item->inode].direct5 = ID_ITEM_FREE;
    (*vfs)->inodes[current_item->inode].indirect1 = ID_ITEM_FREE;
    (*vfs)->inodes[current_item->inode].indirect2 = ID_ITEM_FREE;

    /* Write inode to VFS */
    write_inode_to_vfs(vfs, current_item->inode);

    /* Update parent directory in file */
    update_directory_in_file(vfs, parent, current_item, false);

    free((*vfs)->all_dirs[current_item->inode]);
    free(current_item);
}

/*
 * Remove directory command - parses path and hands over data to further processing
 */
void vfs_rmdir(VFS **vfs, char *path) {
    directory *parent_dir;
    dir_item *item, **item_address;
    bool deleted = false;
    char *name;

    /* Parse path */
    e_return_on_error(parse_path(vfs, path, &name, &parent_dir), PATH_NOT_FOUND_MSG);

    item_address = &(parent_dir->subdir);
    item = parent_dir->subdir;
    while (item != NULL) {
        if (strcmp(name, item->item_name) == 0) {
           remove_dir(vfs, parent_dir, item_address, item);
           deleted = true;
           break;
        }
        item_address = &(item->next);
        item = item->next;
    }

    e_return_on_condition(deleted == false, FILE_NOT_FOUND_MSG);

    printf(OK_MSG);
}

/*
 * Copies command from source path to destination path
 */
void cp(VFS **vfs, char *src_file, char *dest_file) {
    int i, block_count, rest, real_block_count, last_block_size, last_block_index;
    int32_t *source_blocks, *dest_blocks, inode_id;
    char *name, *dest_name;
    directory *source_dir, *dest_dir;
    dir_item *item, **new_item;


    /* Parse source path */
    e_return_on_condition(parse_path(vfs, src_file, &name, &source_dir), FILE_NOT_FOUND_MSG);


    /* Find item in source directory */
    item = find_item(source_dir->file, name);
    e_return_on_null(item, FILE_NOT_FOUND_MSG);

    /* Find the destination directory */
    e_return_on_condition(parse_path(vfs, dest_file, &dest_name, &dest_dir), PATH_NOT_FOUND_MSG);

    if (strlen(dest_name) == 0) {
        dest_name = name;
    }
    // Test if destination folder contains file/directory with the same name
    e_return_on_condition(vfs_exists(dest_dir, dest_name), FILE_EXISTS_MSG);


    /* Get blocks and number of file and required blocks */
    source_blocks = get_data_blocks(vfs, item->inode, &block_count, &rest);
    real_block_count = get_block_count_with_indirect(block_count);

    /* Get free blocks */
    dest_blocks = find_free_data_blocks(vfs, real_block_count);

    e_return_on_null(dest_blocks, NOT_ENOUGH_BLOCKS_MSG);

    /* Get free node */
    inode_id = find_free_inode(vfs);
    e_return_on_error(inode_id, NOT_ENOUGH_BLOCKS_MSG);

    /* Get free item in directory list */
    new_item = &(dest_dir->file);
    while (*new_item != NULL) {
        new_item = &((*new_item)->next);
    }
    *new_item = new_directory_item(inode_id, dest_name);

    int32_t file_size = (*vfs)->inodes[item->inode].file_size;

    /* Create inode for the new file */
    last_block_index = initialize_inode(vfs, inode_id, file_size, block_count, dest_blocks);

    /* Update bitmap, inodes and directories */
    update_bitmap_in_file(vfs, *new_item, 1, dest_blocks, block_count);
    write_inode_to_vfs(vfs, inode_id);
    update_sizes_in_file(vfs, dest_dir, file_size);
    update_directory_in_file(vfs, dest_dir, *new_item, 1);

    char buffer[CLUSTER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    /* Copy data blocks */
    for (i = 0; i < block_count - 1; i++) {
        seek_data_cluster(vfs, source_blocks[i]);
        read_vfs(vfs, buffer, sizeof(buffer), 1);
        seek_data_cluster(vfs, dest_blocks[i]);
        write_vfs(vfs, buffer, sizeof(buffer), 1);
        flush_vfs(vfs);
    }

    /* Copy last data block */
    memset(buffer, 0, CLUSTER_SIZE);
    last_block_size = get_last_block_size(rest);

    seek_data_cluster(vfs, source_blocks[block_count - 1]);
    read_vfs(vfs, buffer, last_block_size, 1);
    seek_data_cluster(vfs, dest_blocks[last_block_index]);
    write_vfs(vfs, buffer, last_block_size, 1);

    free(source_blocks);
    free(dest_blocks);

    printf(OK_MSG);
}

/*
 * Moves file from source path to destination path
 */
void mv(VFS **vfs, char *src_file, char *dest_file) {
    char *name, *dest_name;
    directory *source_dir, *dest_dir;
    dir_item *item, **new_item, **temp;
    bool removed = false;

    /* Parse source path */
    e_return_on_condition(parse_path(vfs, src_file, &name, &source_dir), FILE_NOT_FOUND_MSG);

    /* Find item in source directory */
    item = find_item(source_dir->file, name);
    e_return_on_null(item, FILE_NOT_FOUND_MSG);

    /* Find the destination directory */
    e_return_on_condition(parse_path(vfs, dest_file, &dest_name, &dest_dir), PATH_NOT_FOUND_MSG);

    if (strlen(dest_name) == 0) {
        dest_name = name;
    }

    /* Validate if file exists */
    e_return_on_condition(vfs_exists(dest_dir, dest_name), FILE_EXISTS_MSG);

    /* Delete from source dir */
    temp = &(source_dir->file);
    item = source_dir->file;

    while (item != NULL) {
        if (streq(name, item->item_name)) {
            (*temp) = item->next;
            update_sizes_in_file(vfs, source_dir, -((*vfs)->inodes[item->inode].file_size));
            update_directory_in_file(vfs, source_dir, item, 0);
            removed = true;
            break;
        }
        temp = &(item->next);
        item = item->next;
    }

    e_return_on_condition(removed == false, FILE_NOT_FOUND_MSG);

    /* Update filename */
    memcpy(item->item_name, dest_name, FILENAME_LENGTH);

    /* Find last file in list in destination */
    new_item = &(dest_dir->file);
    while (*new_item != NULL) {
        new_item = &((*new_item)->next);
    }

    *new_item = item;
    update_sizes_in_file(vfs, dest_dir, (*vfs)->inodes[item->inode].file_size);
    update_directory_in_file(vfs, dest_dir, item, 1);

    printf(OK_MSG);
}

/*
 * Loads commands from commands file. Reads the file one by one and processes all the commands.
 */
void load(VFS **vfs, char *filename) {
    FILE *command_file;
    char * line = NULL;;
    size_t len = 0;

    e_return_on_condition(file_exists(filename) == false, FILE_NOT_FOUND_MSG);

    command_file = fopen(filename, "r");
    e_return_on_null(command_file, FILE_NOT_FOUND_MSG);

    while (getline(&line, &len, command_file) != -1) {
        char *input = remove_nl(line);
        if(strlen(input) == 0) {
            continue;
        }

        printf("%s\n", input);
        process_command_line(vfs, input);
    }

    printf(FILE_COMPLETE_MSG);
}


void recursive_inode_check(VFS **vfs, int8_t **inode_indexes, directory *dir) {
    (*inode_indexes)[dir->current->inode] = 0;

    /* Loop through dirs */
    dir_item *item = dir->file;
    while (item != NULL) {
        (*inode_indexes)[item->inode] = 0;
        item = item->next;
    }

    /* Loop through subdirs of the directory */
    item = dir->subdir;
    while (item != NULL) {
        recursive_inode_check(vfs, inode_indexes, (*vfs)->all_dirs[item->inode]);
        item = item->next;
    }
}

void check(VFS **vfs) {
    int real_block_count, number, calculated_block_count;

    bool recursive_result = false;
    int8_t *inode_indexes = calloc(1, (*vfs)->superblock->inode_count);
    for (unsigned long i = 0 ; i < (*vfs)->superblock->inode_count; i++){
        if ((*vfs)->inodes[i].nodeid == ID_ITEM_FREE) {
            continue;
        }

        inode_indexes[i] = 1;
    }

    printf("Assigned inodes (Assigned = 1, Free = 0)\n");
    for (unsigned long i = 0 ; i < (*vfs)->superblock->inode_count; i++){
        printf("%i ", inode_indexes[i]);
    }

    printf("\nInodes after check (Not in directory = 1, All fine = 0)\n");
    recursive_inode_check(vfs, &inode_indexes, (*vfs)->all_dirs[0]);

    for (unsigned long i = 0 ; i < (*vfs)->superblock->inode_count; i++){
        if (inode_indexes[i] == 1) {
            recursive_result = true;
        }
        printf("%i ", inode_indexes[i]);
    }

    printf("\n");
    e_return_on_condition(recursive_result == true, CHECK_FAILED_MSG);

    for (unsigned long i = 0 ; i < (*vfs)->superblock->inode_count; i++){
        inode node = (*vfs)->inodes[i];

        if (node.nodeid != ID_ITEM_FREE) {
            if (node.isDirectory == false) {
                /* Count blocks */
                calculated_block_count = get_block_count_with_indirect((node.file_size / CLUSTER_SIZE) + 1); /* Add 1 to ceil the value */
                real_block_count = 0;
                if (node.direct1 != ID_ITEM_FREE) real_block_count++;
                if (node.direct2 != ID_ITEM_FREE) real_block_count++;
                if (node.direct3 != ID_ITEM_FREE) real_block_count++;
                if (node.direct4 != ID_ITEM_FREE) real_block_count++;
                if (node.direct5 != ID_ITEM_FREE) real_block_count++;

                if (node.indirect1 != ID_ITEM_FREE) {
                    real_block_count++;
                    seek_data_cluster(vfs,node.indirect1);
                    for (i = 0; i < INT32_COUNT_IN_BLOCK; i++) {
                        int32_read_vfs(vfs, &number);
                        if (number == EMPTY_ADDRESS) break;
                        real_block_count++;
                    }
                }

                if (node.indirect2 != ID_ITEM_FREE) {
                    real_block_count++;
                    seek_data_cluster(vfs, node.indirect2);
                    for (i = 0; i < INT32_COUNT_IN_BLOCK; i++) {
                        int32_read_vfs(vfs, &number);
                        if (number == EMPTY_ADDRESS) break;
                        real_block_count++;
                    }
                }

                /* Verify */
                if (calculated_block_count != real_block_count) {
                    printf("Inode ID: %i - Calculated block count: %i, real block count: %i\n", node.nodeid, calculated_block_count, real_block_count);
                    printf(CHECK_FAILED_MSG);
                    return;
                }
            }
        }
    }

    printf(CHECK_SUCCESS_MSG);
}

void size(VFS **vfs, int32_t inode_id, int32_t new_size) {
    (*vfs)->inodes[inode_id].file_size = new_size;

    write_inode_to_vfs(vfs, inode_id);

    printf(OK_MSG);
}
