#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "known_nodes_hashmap.h"
#include "node.h"
#include "logging.h"

struct known_node_hashmap_entry ** known_nodes;

int known_node_hash(char name[NODE_NAME_LEN]) {
    int h = 0;
    int p = 257;
    for (int i = 0; name[i]; i++) {
        h += name[i];
        h *= p;
        h %= HASHMAP_ENTRIES;
    }
    return h;
}

void init_known_nodes_hashmap() {
    known_nodes = calloc(HASHMAP_ENTRIES, sizeof(struct known_node_hashmap_entry *));
}

void add_known_node(struct node * node) {
    int hash = known_node_hash(node->name);
    struct known_node_hashmap_entry ** pos = &known_nodes[hash];
    struct known_node_hashmap_entry * append_to = NULL;
    while (1) {
        if (*pos == NULL || strcmp((*pos)->name, node->name) == 0) {
            break;
        } else {
            append_to = *pos;
            *pos = (*pos)->next;
        }
    }
    if (*pos == NULL) {
        *pos = calloc(1, sizeof(struct known_node_hashmap_entry));
    }
    strcpy((*pos)->name, node->name);
    strcpy((*pos)->ip, node->ip);
    (*pos)->port = node->port;

    log_info("Adding node %s:%s:%u", node->name, node->ip, node->port);

    if (append_to != NULL) {
        append_to->next = *pos;
    }
}

void for_each_known_node(known_node_callback_t callback, void * args) {
    for (int i = 0; i < HASHMAP_ENTRIES; i++) {
        struct known_node_hashmap_entry * current = known_nodes[i];
        while (current != NULL) {
            callback(current, args);
            current = current->next;
        }
    }
}

struct known_node_hashmap_entry * look_up_known_node(char * ip, unsigned short port) {
    for (int i = 0; i < HASHMAP_ENTRIES; i++) {
        struct known_node_hashmap_entry * current = known_nodes[i];
        while (current != NULL) {
            if (strcmp(current->ip, ip) == 0 && port == current->port) return current;
            current = current->next;
        }
    }
    return NULL;
}

void dbg_print_all_known_nodes() {
    for (int i = 0; i < HASHMAP_ENTRIES; i++) {
        struct known_node_hashmap_entry * current = known_nodes[i];
        while (current != NULL) {
            log_debug("%s:%s:%u", current->name, current->ip, current->port);
            current = current->next;
        }
    }
}