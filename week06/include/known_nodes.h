#ifndef _KNOWN_NODES_H
#define _KNOWN_NODES_H

#include "node.h"
#include "globals.h"


struct known_node {
    char name[NODE_NAME_LEN];
    char ip[NODE_NAME_LEN];
    unsigned short port;
    int waiting_pong_since;
};

typedef void (*known_node_callback_t) (struct known_node *, void *);

void init_known_nodes_hashmap();
void add_known_node(struct node * node);
void for_each_known_node(known_node_callback_t callback, void * args);
struct known_node * look_up_known_node(char * ip, unsigned short port);
void get_known_nodes(struct known_node ** arr, size_t * cnt);

#endif