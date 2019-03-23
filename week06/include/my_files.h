#ifndef _MY_FILES_H
#define _MY_FILES_H

#include "globals.h"

struct my_file {
    char filename[KNOWN_FILENAME_LEN];
    char path[MY_PATH_LEN];
};

void init_my_files_hashmap();
void add_my_file(char * filename, char * path);
struct my_file * find_my_file(char * filename);
void get_my_files(struct my_file ** my_files, size_t * cnt);
void load_my_files_from(char * dirname);

#endif
