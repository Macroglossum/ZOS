/*
 * Created by Tomas on 4. 2. 2021.
 */

#include <errno.h>
#include "utils.h"
#include "constants.h"
#include "structures.h"
#include "vfs.h"

/*
 * Returns if string str1 equals str2
 */
bool streq(char *str1, char *str2) {
    if (strcmp(str1, str2) == 0) {
        return true;
    } else {
        return false;
    }
}

/*
 * Returns line from console input
 */
char * get_line() {
    char * line = calloc(1, 100), * linep = line;
    size_t lenmax = 100, len = lenmax;
    int c;

    if(line == NULL) {
        return NULL;
    }

    while(true) {
        c = fgetc(stdin);
        if(c == EOF)
            break;

        if(--len == 0) {
            len = lenmax;
            char * linen = realloc(linep, lenmax *= 2);

            if(linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if((*line++ = c) == '\n')
            break;
    }
    *line = '\0';
    return linep;
}

/*
 * Returns true if string is empty
 */
bool strempty (char *str) {
    if (str == NULL || streq(str, "")) {
        return true;
    }

    return false;
}

/*
 * Checks if file exists
 */
bool file_exists (char *filename) {
    struct stat   buffer;
    return (stat (filename, &buffer) == 0);
}


/*
 * Removes new line characters from string
 */
char *remove_nl (char * message) {
    int i, e, index = 0;
    int len = strlen(message);

    char *new_message = calloc(len, sizeof(char));
    for (i = 0; i < len; i++) {
        e = message[i];

        if (e != 10 && e != 13) {
            new_message[index] = message[i];
            index++;
        }
    }

    new_message[index] = '\0';
    return new_message;
}

/*
 * Coverts size string (e.g. 10K, 10M, 10G) to int (10000, 10000000, 10000000000)
 */
int32_t get_size_from_string(char *string_size) {
    char *str_part = NULL;
    int32_t number;

    if (strempty(string_size)) {
        return ERROR_CODE;
    }

    errno = 0;
    number = strtol(string_size, &str_part, 0);	/*  Convert to number */

    char *units = str_part;

    if (number == 0 || errno != 0) {
        printf(" Error message: %s \n", strerror(errno));
        return ERROR_CODE;
    }

    if (streq("K", units)) {
        number *= 1000;
    }
    else if (streq("M", units)) {
        number *= 1000000;
    }
    else if (streq("G", units)) {
        number *= 1000000000;
    }

    return number;
}

/*
 * Parses file path. Returns directory and filename using pointers. Returns int representing error code
 */
int parse_path(VFS **vfs, char *path, char **name, directory **dir) {
    int length;
    char buff[256];

    if (strempty(path)) {
        return ERROR_CODE;
    }

    if (streq(path, "..")) {
        *name = "";
        *dir = (*vfs)->current_dir->parent;
    } else if ((*name = strrchr(path, '/')) == NULL) {	/* If there is no slash - Only filename/directory */
        *name = path;
        *dir = (*vfs)->current_dir;
    }
    else  {
        length = strlen(path) - strlen(*name);
        if (path[0] == '/') {
            if (!strstr(path + 1, "/")) { /* If there is only root */
                length = 1;
            }

        }

        *name = *name + 1;
        memset(buff, '\0', 256);
        strncpy(buff, path, length);

        // Find the directory
        *dir = find_directory(vfs, buff);
        if (*dir == NULL) {
            return ERROR_CODE;
        }
    }

    return NO_ERROR_CODE;
}

/*
 * Returns true if VFS was formatted. Otherwise returns false and prints message.
 */
bool verify_format(VFS **vfs) {
    if ((*vfs)->is_formatted) {
        return true;
    } else {
        printf(NEEDS_FORMAT_MSG);
        return false;
    }
}

/*
 * Adds string at the beginning of other string
 */
void prepend(char* s, const char* t)
{
    size_t len = strlen(t);
    memmove(s + len, s, strlen(s) + 1);
    memcpy(s, t, len);
}

/*
 * Loops through list of items. Returns item if it matches name, otherwise NULL
 */
dir_item *find_item(dir_item *first_item, char *name) {
    dir_item *item = first_item;

    while (item != NULL) {
        if (streq(name, item->item_name)) {
            break;
        }
        item = item->next;
    }
    return item;
}

/*
 * Prints information about directory or file - Name, filesize, direct and indirect references
 */
void print_dir_item_info(VFS** vfs, dir_item *item) {
    int i;
    int32_t number; // Data block number
    inode node = (*vfs)->inodes[item->inode];

    printf("Name: %s, size: %dB, i-node: %d\n", item->item_name, node.file_size, node.nodeid);
    printf("Direct: ");

    if (node.direct1 != ID_ITEM_FREE) printf("%d ", node.direct1);
    if (node.direct2 != ID_ITEM_FREE) printf("%d ", node.direct2);
    if (node.direct3 != ID_ITEM_FREE) printf("%d ", node.direct3);
    if (node.direct4 != ID_ITEM_FREE) printf("%d ", node.direct4);
    if (node.direct5 != ID_ITEM_FREE) printf("%d ", node.direct5);

    /* Print indirect 1 */
    printf("\nIndirect 1: ");
    if (node.indirect1 != ID_ITEM_FREE) {
        printf("(%d): ", node.indirect1);
        seek_data_cluster(vfs,node.indirect1);
        for (i = 0; i < INT32_COUNT_IN_BLOCK; i++) {
            int32_read_vfs(vfs, &number);
            if (number == EMPTY_ADDRESS) break;
            printf("%d ", number);
        }
    } else {
        printf("FREE");
    }

    /*Print indirect 2 */
    printf("\nIndirect 2: ");
    if (node.indirect2 != ID_ITEM_FREE) {
        printf("(%d) ", node.indirect2);
        seek_data_cluster(vfs, node.indirect2);
        for (i = 0; i < INT32_COUNT_IN_BLOCK; i++) {
            int32_read_vfs(vfs, &number);
            if (number == EMPTY_ADDRESS) break;
            printf("%d ", number);
        }
    } else {
        printf("FREE");
    }
    printf("\n");
}

/*
 * Adjusts number of blocks if indirect references are required
 */
int get_block_count_with_indirect(int block_count) {
    if (block_count < 5) { /* Only direct */
        return block_count;
    } else if ((block_count > 5) && (block_count <= CLUSTER_SIZE / sizeof(int32_t) + 5)) { /* One indirect */
        return block_count + 1;
    } else { /* Two indirects */
        return block_count + 2;
    }
}

/*
 * Returns size of the last Cluster
 */
int get_last_block_size(int rest) {
    if (rest != 0) {
        return rest;
    } else {
        return CLUSTER_SIZE;
    }
}

/*
 * Converts character string to int32
 */
int32_t convert_int(char *str) {
    errno = 0;
    char *end;
    int32_t value = strtol(str, &end, 10);

    if (errno != 0) {
        printf("ERROR: Error converting string '%s' into number! Message: %s \n", str, strerror(errno));
        return INT32_MIN;
    }

    return value;
}