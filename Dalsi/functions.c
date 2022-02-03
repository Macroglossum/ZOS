#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/limits.h>
#include "header.h"
#include <ctype.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>

//1 - not same
//0 - same
int compare_two_string(char *string1, char *string2) {
	char s1[strlen(string1)];
	char s2[strlen(string2)];
	
	strcpy(s1, string1);
	strcpy(s2, string2);

	if (strlen(s1) > 0 && s1[strlen(s1) - 1] == '\n') s1[strlen(s1) - 1] = '\0';
	if (strlen(s2) > 0 && s2[strlen(s2) - 1] == '\n') s2[strlen(s2) - 1] = '\0';

	if (strlen(s1) != strlen(s2)) return 1;
	else {
		int i;
		for (i = 0; i < strlen(s1); i++) {
			if (s1[i] != s2[i]) return 1;
		}
		return 0;
	}
}

int array_length_strtok(char *path) {
	char str[PATH_MAX];
	strcpy(str, path);

	char *tok = strtok(str, "/");
	int i = 1;
	while(1) {
		if ((tok = strtok(NULL, "/")) == NULL) break;
		else i++;
	}
	return i;
}

int index_of_last_digit(char *size) {
	char str[strlen(size)];
	strcpy(str, size);
	
	int index = 0;
	while(isdigit(str[index])) {
		index++;
	}

	return index;
}

int get_multiple(char *multiple, int size) {
	int multiple_number = 1;
	char mult[size];
	strncpy(mult, multiple, size);

	switch(size) {
		case 2:
			switch(mult[0]) {
				case 'K':
					multiple_number = 1000;
					break;
				case 'M':
					multiple_number = 1000 * 1000;
					break;
				case 'G':
					multiple_number = 1000 * 1000 * 1000;
					break;
				default:
					printf("Wrong multiple, using only B\n");
					break;
			}
			break;
		default:
			printf("Wrong multiple, using only B\n");
			break;
	}

	return multiple_number;
}

int directory_exists(char *path) {

	struct stat s;
	int err = stat(path, &s);
	if(-1 == err) {
		if(ENOENT == errno) {
			return 0;
		}
		else return 0;
	} 
	else {
		if(S_ISDIR(s.st_mode)) {
			return 1;
		} 
		else {
			return 0;	
		}
	}
	return 0;
}

int my_atoi(const char* snum) {
    int idx, strIdx = 0, accum = 0, numIsNeg = 0;
    const unsigned int NUMLEN = (int)strlen(snum);

    if(snum[0] == 0x2d)
        numIsNeg = 1;

    for(idx = NUMLEN - 1; idx >= 0; idx--)
    {
        if(snum[strIdx] >= 0x30 && snum[strIdx] <= 0x39)
            accum += (snum[strIdx] - 0x30) * pow(10, idx);

        strIdx++;
    }

    if(!numIsNeg)
        return accum;
    else
        return accum * -1;
}
