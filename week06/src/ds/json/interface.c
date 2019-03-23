/*
 * Да, я веб-макака и написал кастомную реализацию JSON'а на C
 */

#include <string.h>

#include "include/ds/json.h"

#define HASHMAP_ENTRIES 63

int json_hash_fn(struct json_kv_pair * value) {
    return polynomial_hash(value->property, 257, HASHMAP_ENTRIES);
}
int json_hash_compare_fn(struct json_kv_pair * a, struct json_kv_pair * b) {
    return strcmp(a->property, b->property);
}

json_t * create_json_null() {
    json_t * null = calloc(1, sizeof(json_t));
    null->type = JSON_NULL;
    return null;
}

json_t * create_json_boolean(int value) {
    json_t * boolean = calloc(1, sizeof(json_t));
    boolean->type = JSON_BOOLEAN;
    boolean->int_value = value;
    return boolean;
}

json_t * create_json_integer(int value) {
    json_t * integer = calloc(1, sizeof(json_t));
    integer->type = JSON_INTEGER;
    integer->int_value = value;
    return integer;
}

json_t * create_json_string(char * data) {
    json_t * string = calloc(1, sizeof(json_t));
    string->type = JSON_STRING;
    string->string_value = calloc(strlen(data) + 1, 1);
    strcpy(string->string_value, data);
    return string;
}

json_t * create_json_array() {
    json_t * array = calloc(1, sizeof(json_t));
    array->type = JSON_ARRAY;
    array->array_value = vector_create();
    return array;
}

json_t * create_json_object() {
    json_t * object = calloc(1, sizeof(json_t));
    object->type = JSON_OBJECT;
    object->object_value = hashmap_create(HASHMAP_ENTRIES, (hashmap_hash_fn) json_hash_fn, (hashmap_compare_fn) json_hash_compare_fn);
    return object;
}

json_t * json_array_index(json_t * array, uint32_t index) {
    return vector_index(array->array_value, index);
}

void json_array_append(json_t * array, json_t * value) {
    vector_append(array->array_value, value);
}

void json_array_remove(json_t * array, uint32_t index) {
    vector_remove(array->array_value, index);
}

uint32_t json_array_length(json_t * array) {
    return vector_length(array->array_value);
}

json_t * json_object_to_array(json_t * object) {  // TODO: refactor into using only public hashmap interface
    if (object->type != JSON_OBJECT) {
        return NULL;
    }
    json_t * array = create_json_array();
    for (int i = 0; i < HASHMAP_ENTRIES; i++) {
        struct hashmap_entry * current = object->object_value->data[i];
        while (current != NULL) {
            json_t * mini_array = create_json_array();
            struct json_kv_pair kv_pair = *((struct json_kv_pair *) current->value);
            json_array_append(mini_array, create_json_string(kv_pair.property));
            json_array_append(mini_array, kv_pair.value);
            json_array_append(array, mini_array);
            current = current->next;
        }
    }
    return array;
}

json_t * json_get_property(json_t * object, char * property) {
    if (object->type != JSON_OBJECT) {
        return NULL;
    }
    struct json_kv_pair mock;
    mock.property = property;
    struct hashmap_entry * value = hashmap_look_up_element(object->object_value, &mock, NULL);
    if (value) {
        return ((struct json_kv_pair *) value->value)->value;
    } else {
        return NULL;
    }
}

void json_set_property(json_t * object, char * property, json_t * value) {
    if (object->type != JSON_OBJECT) {
        return;
    }
    struct json_kv_pair * entry = calloc(1, sizeof(struct json_kv_pair));
    entry->property = calloc(strlen(property + 1), 1);
    strcpy(entry->property, property);
    entry->value = value;
    hashmap_add_element(object->object_value, entry);
}

void json_remove_property(json_t * object, char * property) {

}

void json_destroy(json_t * value) {
    switch (value->type) {
        case JSON_NULL:
        case JSON_BOOLEAN:
        case JSON_INTEGER:
            break;
        case JSON_STRING:
            free(value->string_value);
            break;
        case JSON_ARRAY:
            for (uint32_t i = 0; i < json_array_length(value); i++) {
                json_destroy(json_array_index(value, i));
            }
            free(value->array_value);
            break;
        case JSON_OBJECT:
            ; // nop for parser to be adequate (THANKS CLION)
            json_t * array = json_object_to_array(value);
            json_destroy(array);
            hashmap_destroy(value->object_value);
            break;
    }
}