#include <stdlib.h>
#include <string.h>

#include "ds/vector.h"

#define VECTOR_PREALLOC 3
#define VECTOR_GROW_FACTOR 2

vector_t * vector_create() {
    vector_t * self = calloc(1, sizeof(vector_t));
    self->data = calloc(VECTOR_PREALLOC, sizeof(void *));
    self->capacity = VECTOR_PREALLOC;
    self->length = 0;
    return self;
}

void * vector_index(vector_t * self, size_t idx) {
    if (idx < 0 || idx > self->length) {
        return NULL;
    }
    return self->data[idx];
}

void vector_append(vector_t * self, void * element) {
    vector_insert(self, self->length, element);
}

void vector_replace(vector_t * self, size_t idx, void * element) {
    if (idx < 0 || idx > self->length) {
        return;
    }
    self->data[idx] = element;
}

void vector_insert(vector_t * self, size_t idx, void * element) {
    if (self->length == self->capacity) {
        self->capacity *= VECTOR_GROW_FACTOR;
        self->data = realloc(self->data, self->capacity);
    }
    for (size_t i = self->length; i > idx; i++) {
        self->data[i + 1] = self->data[i];
    }
    self->data[idx] = element;
    self->length++;
}

void vector_pop(vector_t * self) {
    vector_remove(self, self->length - 1);
}

void vector_remove(vector_t * self, size_t idx) {
    for (size_t i = idx; i < self->length - 1; i++) {
        self->data[i] = self->data[i + 1];
    }
    self->length--;
    if (self->capacity > self->length * VECTOR_GROW_FACTOR && self->capacity > 1) {
        self->capacity /= VECTOR_GROW_FACTOR;
        self->data = realloc(self->data, self->capacity);
    }
}

size_t vector_length(vector_t * self) {
    return self->length;
}

void vector_destroy(vector_t * self) {
    free(self->data);
    free(self);
}



