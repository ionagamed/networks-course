#include <string.h>
#include <stdio.h>

#include "logging.h"
#include "known_files.h"
#include "include/ds/hashmap.h"

struct hashmap * known_files_hashmap;

int known_file_hash_fn(struct known_file * element) {
    return polynomial_hash(element->filename, 257, HASHMAP_ENTRIES);
}

int known_file_compare_fn(struct known_file * a, struct known_file * b) {
    return strcmp(a->filename, b->filename);
}

void init_known_files_hashmap() {
    known_files_hashmap = hashmap_create(
            HASHMAP_ENTRIES,
            (hashmap_hash_fn) known_file_hash_fn,
            (hashmap_compare_fn) known_file_compare_fn
    );
}

void add_known_file(char * filename, struct node * node) {
    struct known_file * el = calloc(1, sizeof(struct known_file));
    if (el == NULL) {
        log_debug("Failed to allocate memory");
        perror("calloc");
        exit(-1);
    }
    strcpy(el->filename, filename);
    el->node = calloc(1, sizeof(struct node));
    memcpy(el->node, node, sizeof(struct node));
    log_debug("Adding remote file %s (through node %s:%s:%u)", filename, node->name, node->ip, node->port);
    hashmap_add_element(known_files_hashmap, el);
}

void get_known_files(struct known_file ** known_files, size_t * cnt) {
    hashmap_get_entries(known_files_hashmap, (void **) known_files, cnt);
}

struct known_file * find_known_file(char * filename) {
    struct known_file mock;
    strcpy(mock.filename, filename);
    struct hashmap_entry * entry = hashmap_look_up_element(known_files_hashmap, &mock, NULL);
    if (entry != NULL) {
        return entry->value;
    } else {
        return NULL;
    }
}