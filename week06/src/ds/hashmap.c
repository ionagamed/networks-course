#include <stdio.h>

#include "logging.h"
#include "include/ds/hashmap.h"

struct hashmap * hashmap_create(size_t size, hashmap_hash_fn hash_fn, hashmap_compare_fn compare_fn) {
    struct hashmap * ret = calloc(1, sizeof(struct hashmap));
    if (ret == NULL) {
        log_debug("Failed to allocate memory");
        perror("calloc");
        exit(-1);
    }
    ret->size = size;
    ret->hash_fn = hash_fn;
    ret->compare_fn = compare_fn;
    ret->data = calloc(size, sizeof(void *));
    if (ret->data == NULL) {
        log_debug("Failed to allocate memory");
        perror("calloc");
        exit(-1);
    }
    return ret;
}

void hashmap_add_element(struct hashmap * self, void * element) {
    int hash = self->hash_fn(element);
    struct hashmap_entry ** pos = &self->data[hash];
    while (1) {
        if (*pos == NULL || self->compare_fn(*pos, element) == 0) {
            break;
        } else {
            pos = &((*pos)->next);
        }
    }
    if (*pos == NULL) {
        *pos = calloc(1, sizeof(struct hashmap_entry));
        if (*pos == NULL) {
            log_debug("Failed to allocate memory");
            perror("calloc");
            exit(-1);
        }
    }
    (*pos)->value = element;
}

void hashmap_remove_element(struct hashmap * self, void * element) {
    int hash = self->hash_fn(element);
    struct hashmap_entry ** pos = &self->data[hash];
    while (1) {
        if (*pos == NULL || self->compare_fn(*pos, element) == 0) {
            break;
        } else {
            pos = &((*pos)->next);
        }
    }
    if (*pos == NULL) {
        return;
    }
    struct hashmap_entry * old_ptr = *pos;
    *pos = (*pos)->next;
    free(old_ptr);
}

void hashmap_for_each_element(struct hashmap * self, hashmap_callback_fn callback, void * args) {
    for (size_t i = 0; i < self->size; i++) {
        struct hashmap_entry * current = self->data[i];
        while (current != NULL) {
            callback(current->value, args);
            current = current->next;
        }
    }
}

struct hashmap_entry * hashmap_look_up_element(struct hashmap * self, void * element, hashmap_compare_fn compare_fn) {
    if (compare_fn == NULL) compare_fn = self->compare_fn;
    for (size_t i = 0; i < self->size; i++) {
        struct hashmap_entry * current = self->data[i];
        while (current != NULL) {
            if (compare_fn(element, current->value) == 0) {
                return current;
            }
            current = current->next;
        }
    }
    return NULL;
}

void hashmap_get_entries(struct hashmap * self, void ** entries, size_t * cnt) {
    *cnt = 0;
    for (int i = 0; i < self->size; i++) {
        struct hashmap_entry * current = self->data[i];
        while (current != NULL) {
            entries[*cnt] = current->value;
            (*cnt)++;
            current = current->next;
        }
    }
}

void hashmap_destroy(struct hashmap * self) {
    free(self->data);
    free(self);
}

int polynomial_hash(const char * s, int p, int m) {
    int h = 0;
    for (int i = 0; s[i]; i++) {
        h += s[i];
        h *= p;
        h %= m;
    }
    return h;
}