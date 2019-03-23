#include "common.h"

struct hashmap * refcounter_hashmap;

struct refcounter_entry {
    void * ptr;
    int refs;
};

int refcounter_hash_fn(struct refcounter_entry * element) {
    return ((int)element->ptr) % HASHMAP_ENTRIES;
}

int refcounter_compare_fn(struct refcounter_entry * a, struct refcounter_entry * b) {
    return a->ptr == b->ptr;
}

void init_refcounter() {
    refcounter_hashmap = hashmap_create(HASHMAP_ENTRIES, refcounter_hash_fn, refcounter_compare_fn);
}

void incref(void * ptr) {
    struct refcounter_entry * mock = calloc(1, sizeof(struct refcounter_entry));
    mock->ptr = ptr;
    struct hashmap_entry * entry = hashmap_look_up_element(refcounter_hashmap, mock, NULL);
    if (entry) {
        ((struct refcounter_entry *) entry->value)->refs += 1;
        free(mock);
    } else {
        mock->refs = 1;
        hashmap_add_element(refcounter_hashmap, mock);
    }
}

void decref(void * ptr) {
    struct refcounter_entry * mock = calloc(1, sizeof(struct refcounter_entry));
    mock->ptr = ptr;
    struct hashmap_entry * entry = hashmap_look_up_element(refcounter_hashmap, mock, NULL);
    if (entry) {
        struct refcounter_entry * value = entry->value;
        value->refs -= 1;
        free(mock);
        if (value->refs == 0) {
            free(value->ptr);
            hashmap_remove_element(refcounter_hashmap, value);
        }
    }
}