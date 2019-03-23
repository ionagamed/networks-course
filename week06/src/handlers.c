#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>

#include "handlers.h"
#include "event_loop.h"
#include "my_files.h"
#include "known_files.h"
#include "common.h"
#include "known_nodes.h"
#include "file_transfer.h"

void push_node(struct known_node * to, struct node * node) {
    json_t * payload = serialize_node(node);
    json_t * push_request = pack("push", payload);
    send_request_plain(to->ip, to->port, push_request);
    json_destroy(push_request);
}

HANDLER(hello) {
    json_t * response = pack("hello_response", create_json_null());

    struct node client_node;
    strcpy(client_node.ip, json_get_property(from, "ip")->string_value);
    strncpy(client_node.name, json_get_property(from, "name")->string_value, NODE_NAME_LEN);
    client_node.name[NODE_NAME_LEN - 1] = 0;
    client_node.port = json_get_property(from, "listen_port")->int_value;
    for_each_known_node((known_node_callback_t) push_node, &client_node);
    add_known_node(&client_node);
    *keep_alive = 1;
    return response;
}

HANDLER(get_node_list) {
    json_t * node_list = create_json_array();

    size_t cnt_size = 0;
    struct known_node * known_nodes[HASHMAP_ENTRIES];
    get_known_nodes(known_nodes, &cnt_size);
    int cnt = (int) cnt_size;

    struct node n;
    for (int i = 0; i < cnt; i++) {
        log_debug("Sending node %s (idx %d)", known_nodes[i]->name, i);
        strncpy(n.name, known_nodes[i]->name, NODE_NAME_LEN);
        n.name[NODE_NAME_LEN - 1] = 0;
        strcpy(n.ip, known_nodes[i]->ip);
        n.port = known_nodes[i]->port;
        json_array_append(node_list, serialize_node(&n));
    }

    *keep_alive = 1; // TODO: use flag from client

    return pack("node_list_response", node_list);
}

HANDLER(ping) {
    return pack("pong", create_json_null());
}

HANDLER(push_node) {
    struct node * pushed_node = deserialize_node(payload);
    add_known_node(pushed_node);
    free(pushed_node);
    return NULL;
}

HANDLER(get_file_list) {
    json_t * response = create_json_array();

    size_t known_files_cnt = 0;
    struct known_file * known_files[HASHMAP_ENTRIES];
    get_known_files(known_files, &known_files_cnt);

    size_t my_files_cnt = 0;
    struct my_file * my_files[HASHMAP_ENTRIES];
    get_my_files(my_files, &my_files_cnt);

    for (int i = 0; i < known_files_cnt; i++) {
        json_t * file = create_json_object();
        json_set_property(file, "filename", create_json_string(known_files[i]->filename));
        json_set_property(file, "node", serialize_node(known_files[i]->node));
        json_array_append(response, file);
    }

    for (int i = 0; i < my_files_cnt; i++) {
        json_t * file = create_json_object();
        json_set_property(file, "filename", create_json_string(my_files[i]->filename));
        json_array_append(response, file);
    }

    return pack("file_list_response", response);
}

HANDLER(get_file) {
    json_t * response = create_json_object();

    json_t * filename = json_get_property(payload, "filename");
    struct my_file * my_file = find_my_file(filename->string_value);

    if (!my_file) {
        json_set_property(response, "status", create_json_string("error"));
        json_set_property(response, "message", create_json_string("no such file"));
        return response;
    }

    begin_file_upload(json_get_property(from, "socket")->int_value, my_file->path);

    *keep_alive = 1;
    return NULL;
}


#define USE_HANDLER(magic, handler) if (strcmp(type, #magic) == 0) {log_debug("invoking " #magic " handler"); response_body = handler(payload, keep_alive, from);}
#define USE_DEFAULT_HANDLER(type) USE_HANDLER(type, type##_handler)
json_t * on_client_request(json_t * request, int * keep_alive) {
    json_t * body = json_get_property(request, "body");
    json_t * from = json_get_property(request, "from");
    char * type = json_get_property(body, "type")->string_value;
    json_t * payload = json_get_property(body, "payload");

    json_t * response_body;

    log_debug("Request: %s\n", serialize_json(request));

    USE_DEFAULT_HANDLER(hello);
    USE_DEFAULT_HANDLER(get_node_list);
    USE_DEFAULT_HANDLER(ping);
    USE_DEFAULT_HANDLER(push_node);
    USE_DEFAULT_HANDLER(get_file_list);
    USE_DEFAULT_HANDLER(get_file);

    if (response_body != NULL) {
        json_t * response = create_json_object();
        json_set_property(response, "from", pack_from());
        json_set_property(response, "body", response_body);

        log_debug("Response: %s\n", serialize_json(response));

        return response;
    } else {
        return NULL;
    }
}
#undef USE_DEFAULT_HANDLER
#undef USE_HANDLER