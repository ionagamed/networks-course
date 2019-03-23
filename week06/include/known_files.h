#ifndef _FILE_HASHMAP_H
#define _FILE_HASHMAP_H

#include "node.h"
#include "globals.h"

struct known_file {
    char filename[KNOWN_FILENAME_LEN];
    struct node * node;
};
void init_known_files_hashmap();
void add_known_file(char * filename, struct node * node);
void get_known_files(struct known_file ** known_files, size_t * cnt);
struct known_file * find_known_file(char * filename);


#endif
