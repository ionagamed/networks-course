#ifndef _HASHMAP_H
#define _HASHMAP_H

#include <stdlib.h>

typedef int (*hashmap_hash_fn) (void * element);
typedef int (*hashmap_compare_fn) (void * a, void * b);
typedef void (*hashmap_callback_fn) (void * element, void * args);

struct hashmap_entry {
    void * value;
    struct hashmap_entry * next;
};

struct hashmap {
    struct hashmap_entry ** data;
    size_t size;
    hashmap_hash_fn hash_fn;
    hashmap_compare_fn compare_fn;
};

struct hashmap * hashmap_create(size_t size, hashmap_hash_fn hash_fn, hashmap_compare_fn compare_fn);
void hashmap_add_element(struct hashmap * self, void * element);
void hashmap_remove_element(struct hashmap * self, void * element);
void hashmap_for_each_element(struct hashmap * self, hashmap_callback_fn callback, void * args);
struct hashmap_entry * hashmap_look_up_element(struct hashmap * self, void * element, hashmap_compare_fn compare_fn);
void hashmap_get_entries(struct hashmap * self, void ** entries, size_t * cnt);
void hashmap_destroy(struct hashmap * self);

int polynomial_hash(const char * s, int p, int m);


#endif
