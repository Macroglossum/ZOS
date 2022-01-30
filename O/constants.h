/*
 * Created by Tomas Linhart on 4. 2. 2021.
 */

#ifndef SP_CONSTANTS_H
#define SP_CONSTANTS_H

#define SUPERBLOCK_SIGNATURE "Tomas Linhart"
#define SIGNATURE_LENGTH 20

#define CLUSTER_SIZE 4096
#define INT32_COUNT_IN_BLOCK CLUSTER_SIZE / 4
#define MIN_FS 102400
#define FILENAME_LENGTH 12
#define NEGATIVE_SIZE_OF_INT32 -4
#define INODE_SIZE 38 /* Using sizeof returns 40 - why? */
#define ID_ITEM_FREE -1
#define EMPTY_ADDRESS 0

#define ERROR_CODE -1
#define NO_ERROR_CODE 0

#define EXIT_COMMAND "exit"
#define HELP_COMMAND "help"
#define FORMAT_COMMAND "format"
#define DEBUG_COMMAND "debug"
#define INCP_COMMAND "incp"
#define OUTCP_COMMAND "outcp"
#define MKDIR_COMMAND "mkdir"
#define CD_COMMAND "cd"
#define LS_COMMAND "ls"
#define CAT_COMMAND "cat"
#define PWD_COMMAND "pwd"
#define INFO_COMMAND "info"
#define RM_COMMAND "rm"
#define RMDIR_COMMAND "rmdir"
#define CP_COMMAND "cp"
#define MV_COMMAND "mv"
#define LOAD_COMMAND "load"
#define CHECK_COMMAND "check"
#define SIZE_COMMAND "size"

#define PATH_DELIMETER "/"

#define PATH_NOT_FOUND_MSG "PATH NOT FOUND (neexistuje cílová cesta)\n"
#define FILE_NOT_FOUND_MSG "FILE NOT FOUND (není zdroj)\n"
#define FILE_EXISTS_MSG "EXIST (nelze založit, již existuje)\n"
#define DIR_NOT_EMPTY_MSG "NOT EMPTY (adresář obsahuje podadresáře, nebo soubory)\n"
#define OK_MSG "OK\n"

#define FORMAT_ERROR_SIZE_MSG "Cannot format, too small filesystem. Specify size at least 102400 bits.\n"
#define FORMAT_ERROR_MSG "Cannot format, wrong size specified.\n"
#define MEMORY_ERROR_MSG "Cannot allocate more memory, returning.\n"
#define OPEN_FILE_ERR_MSG "Cannot open file for this filesystem. Returning.\n"
#define FORMAT_SUCCESS_MSG "Formatting successful!\n"
#define VFS_LOAD_SUCCESS "VFS loaded successfully!\n"
#define NEEDS_FORMAT_MSG "You have to format VFS first.\n"
#define START_NEEDS_FORMAT_MSG "VFS File not found, needs formatting.\n"
#define SRC_NOT_DEFINED_MSG "Source file not defined.\n"
#define DEST_NOT_DEFINED_MSG "Destination file not defined.\n"
#define NOT_ENOUGH_BLOCKS_MSG "Not enough blocks found. Probably no more space available. \n"
#define NO_FREE_INODE_MSG "No free inode found. Probably no more space available. \n"
#define UNKNOWN_COMMAND_MSG "Unknown command: %s\n"
#define FILENAME_TOO_LONG_MSG "Filename of the file can be max 12 characters. \n"
#define FILE_COMPLETE_MSG "Sequence of commands from file is completed. \n"
#define CHECK_FAILED_MSG "Consistency check failed!\n"
#define CHECK_SUCCESS_MSG "Consistency check successful\n"
#define SIZE_WRONG_INODE_MSG "Could not convert I-node ID to char\n"
#define FILESIZE_CONVERT_ERROR "Could not convert filesize to int\n"

#endif //SP_CONSTANTS_H
