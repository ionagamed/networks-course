#ifndef _KNOWN_NODES_H
#define _KNOWN_NODES_H

#include "node.h"

#define HASHMAP_ENTRIES 63

extern struct known_node_hashmap_entry ** known_nodes;

typedef void (*known_node_callback_t) (struct known_node_hashmap_entry *, void *);

struct known_node_hashmap_entry {
    char name[20];
    char ip[20];
    unsigned short port;
    int waiting_pong_since;
    struct known_node_hashmap_entry * next;
};

int known_node_hash(char ip[20]);
void init_known_nodes_hashmap();
void add_known_node(struct node * node);
void for_each_known_node(known_node_callback_t callback, void * args);
struct known_node_hashmap_entry * look_up_known_node(char * ip, unsigned short port);

void dbg_print_all_known_nodes();

#endif