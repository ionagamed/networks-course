#include "serializers.h"

json_t * serialize_node(struct node * node) {
    json_t * data = create_json_object();
    json_set_property(data, "name", create_json_string(node->name));
    json_set_property(data, "ip", create_json_string(node->ip));
    json_set_property(data, "listen_port", create_json_integer(node->port));
    return data;
}

struct node * deserialize_node(json_t * data) {
    struct node * node = calloc(1, sizeof(struct node));
    strcpy(node->name, json_get_property(data, "name")->string_value);
    strcpy(node->ip, json_get_property(data, "ip")->string_value);
    node->port = json_get_property(data, "listen_port")->int_value;
    return node;
}