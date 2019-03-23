#ifndef _JSON_H
#define _JSON_H

#include <stdlib.h>
#include "include/ds/hashmap.h"
#include "ds/vector.h"

enum {
    JSON_NULL,
    JSON_BOOLEAN,
    JSON_INTEGER,
    JSON_STRING,
    JSON_OBJECT,
    JSON_ARRAY
};

typedef struct {
    uint8_t type;

    int int_value;

    char * string_value;

    struct hashmap * object_value;

    vector_t * array_value;
} json_t;

struct json_kv_pair {
    char * property;
    json_t * value;
};

char * serialize_json(json_t * data);
json_t * deserialize_json(char * data, uint32_t data_len);

json_t * create_json_null();
json_t * create_json_boolean(int value);
json_t * create_json_integer(int value);
json_t * create_json_string(char * data);
json_t * create_json_object();
json_t * create_json_array();

json_t * json_array_index(json_t * array, uint32_t index);
void json_array_append(json_t * array, json_t * value);
void json_array_remove(json_t * array, uint32_t index);
uint32_t json_array_length(json_t * array);

json_t * json_object_to_array(json_t * object);
json_t * json_get_property(json_t * object, char * property);
void json_set_property(json_t * object, char * property, json_t * value);
void json_remove_property(json_t * object, char * property);

void json_destroy(json_t * value);

#endif
