#include <string.h>
#include <stdio.h>
#include <dirent.h>

#include "my_files.h"
#include "include/ds/hashmap.h"
#include "logging.h"

struct hashmap * my_files_hashmap;

int my_file_hash_fn(struct my_file * element) {
    return polynomial_hash(element->filename, 257, HASHMAP_ENTRIES);
}

int my_file_compare_fn(struct my_file * a, struct my_file * b) {
    return strcmp(a->filename, b->filename);
}

void init_my_files_hashmap() {
    my_files_hashmap = hashmap_create(
            HASHMAP_ENTRIES,
            (hashmap_hash_fn) my_file_hash_fn,
            (hashmap_compare_fn) my_file_compare_fn
    );
}

void add_my_file(char * filename, char * path) {
    struct my_file * my_file = calloc(1, sizeof(struct my_file));
    if (my_file == NULL) {
        log_debug("Failed to allocate memory");
        perror("calloc");
        exit(-1);
    }
    log_debug("Adding local file %s (with path %s)", filename, path);
    strcpy(my_file->filename, filename);
    strcpy(my_file->path, path);
    hashmap_add_element(my_files_hashmap, my_file);
}

struct my_file * find_my_file(char * filename) {
    struct my_file mock;
    strcpy(mock.filename, filename);
    struct hashmap_entry * entry = hashmap_look_up_element(my_files_hashmap, &mock, NULL);
    if (entry != NULL) {
        return entry->value;
    } else {
        return NULL;
    }
}

void load_my_files_from(char * dirname) {
    struct dirent * dirent;
    DIR * dir;
    dir = opendir(dirname);
    if (dir) {
        while ((dirent = readdir(dir)) != NULL) {
            if (strcmp(dirent->d_name, ".") != 0 && strcmp(dirent->d_name, "..") != 0) {
                char full_path[BUFFER_SIZE];
                sprintf(full_path, "%s/%s", dirname, dirent->d_name);
                add_my_file(dirent->d_name, full_path);
            }
        }
    }
    closedir(dir);
}

void get_my_files(struct my_file ** my_files, size_t * cnt) {
    hashmap_get_entries(my_files_hashmap, (void **) my_files, cnt);
}