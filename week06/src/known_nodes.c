#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "known_nodes.h"
#include "include/ds/hashmap.h"
#include "node.h"
#include "logging.h"

struct hashmap *known_nodes_hashmap;

int known_nodes_hash_fn(struct known_node *el) {
    return polynomial_hash(el->name, 257, HASHMAP_ENTRIES);
}

int known_nodes_compare_fn(struct known_node *a, struct known_node * b) {
    return strcmp(a->name, b->name);
}

void init_known_nodes_hashmap() {
    known_nodes_hashmap = hashmap_create(
            HASHMAP_ENTRIES,
            (hashmap_hash_fn) known_nodes_hash_fn,
            (hashmap_compare_fn) known_nodes_compare_fn
    );
}

void add_known_node(struct node * node) {
    struct known_node * known_node = calloc(1, sizeof(struct known_node));
    if (known_node == NULL) {
        log_debug("Failed to allocate memory");
        perror("calloc");
        exit(-1);
    }
    strcpy(known_node->name, node->name);
    strcpy(known_node->ip, node->ip);
    known_node->port = node->port;
    log_info("Adding node %s:%s:%u", node->name, node->ip, node->port);
    hashmap_add_element(known_nodes_hashmap, known_node);
}

void for_each_known_node(known_node_callback_t callback, void * args) {
    hashmap_for_each_element(known_nodes_hashmap, (hashmap_callback_fn) callback, args);
}

int ip_and_port_known_node_compare_fn(struct known_node * a, struct known_node * b) {
    int r_ip = strcmp(a->ip, b->ip);
    if (r_ip != 0) return r_ip;
    return a->port - b->port;
}

struct known_node * look_up_known_node(char * ip, unsigned short port) {
    struct known_node mock;
    strcpy(mock.ip, ip);
    mock.port = port;
    struct hashmap_entry * entry = hashmap_look_up_element(known_nodes_hashmap, &mock, (hashmap_compare_fn) ip_and_port_known_node_compare_fn);
    if (entry) {
        return entry->value;
    } else {
        return NULL;
    }
}

void get_known_nodes(struct known_node ** arr, size_t * cnt) {
    hashmap_get_entries(known_nodes_hashmap, (void **) arr, cnt);
}