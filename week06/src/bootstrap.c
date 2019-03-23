#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "known_nodes.h"
#include "bootstrap.h"
#include "common.h"
#include "known_files.h"

int b_get_known_nodes(int sock, char * ip, struct node ** nodes, size_t * nodes_count) {
    json_t * request = pack("get_node_list", create_json_null());
    json_t * response = send_receive_sock(sock, ip, request, NULL);

    json_t * payload = json_get_property(response, "payload");
    int cnt = json_array_length(payload);
    log_debug("Got %d nodes from bootstrap", cnt);

    *nodes = calloc(cnt, sizeof(struct node));
    *nodes_count = cnt;
    if (*nodes == NULL) {
        log_debug("Failed to allocate memory");
        perror("calloc");
        exit(-1);
    }
    for (int i = 0; i < cnt; i++) {
        struct node * node = deserialize_node(json_array_index(payload, i));
        memcpy(*nodes + i, node, sizeof(struct node));
        free(node);
    }
    json_destroy(response);

    return 0;
}

int b_add_known_files(int sock, char * ip) {
    json_t * request = pack("get_file_list", create_json_null());
    json_t * from;
    json_t * response = send_receive_sock(sock, ip, request, &from);
    if (response == NULL) {
        return -1;
    }
    struct node b_node;
    strcpy(b_node.ip, ip);
    strcpy(b_node.name, json_get_property(from, "name")->string_value);
    b_node.port = json_get_property(from, "listen_port")->int_value;
    json_t * payload = json_get_property(response, "payload");
    int cnt = json_array_length(payload);

    for (int i = 0; i < cnt; i++) {
        struct node * node;
        json_t * file = json_array_index(payload, i);
        if (json_get_property(file, "node")) {
            node = deserialize_node(json_get_property(file, "node"));
        } else {
            node = &b_node;
        }
        char * filename = json_get_property(file, "filename")->string_value;
        add_known_file(filename, node);
    }
    json_destroy(response);

    return 0;
}

int say_hello(int sock, char * ip, char * name, unsigned short port, char * out_name, unsigned short * out_port) {
    json_t * request = pack("hello", create_json_null());
    json_t * from;
    send_receive_sock(sock, ip, request, &from);
    *out_port = json_get_property(from, "listen_port")->int_value;
    strcpy(out_name, json_get_property(from, "name")->string_value);
    return 0;
}

int bootstrap_known_nodes(char * bootstrap_ip, unsigned short bootstrap_port, unsigned short listen_port) {
    struct node * nodes;
    size_t nodes_count;

    int sock = connect_plain(bootstrap_ip, bootstrap_port);
    if (sock < 0) {
        return -1;
    }

    struct node bootstrap_node;
    unsigned short bootstrap_server_port;
    say_hello(sock, bootstrap_ip, node_name, listen_port, bootstrap_node.name, &bootstrap_server_port);
    bootstrap_node.port = bootstrap_server_port;
    strcpy(bootstrap_node.ip, bootstrap_ip);
    add_known_node(&bootstrap_node);

    if (b_get_known_nodes(sock, bootstrap_ip, &nodes, &nodes_count) != 0) {
        return -1;
    }
    for (int i = 0; i < nodes_count; i++) {
        if (strcmp(nodes[i].name, node_name) != 0) { // don't add yourself
            add_known_node(&nodes[i]);
        }
    }
    if (b_add_known_files(sock, bootstrap_ip) != 0) {
        return -1;
    }

    return 0;
}