#ifndef _VECTOR_H
#define _VECTOR_H

#include <stdlib.h>

typedef struct {
    void ** data;
    size_t length;
    size_t capacity;
} vector_t;

vector_t * vector_create();

void * vector_index(vector_t * self, size_t idx);

void vector_append(vector_t * self, void * element);
void vector_replace(vector_t * self, size_t idx, void * element);
void vector_insert(vector_t * self, size_t idx, void * element);

void vector_pop(vector_t * self);
void vector_remove(vector_t * self, size_t idx);

size_t vector_length(vector_t * self);

void vector_destroy(vector_t * self);

#endif
