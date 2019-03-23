#ifndef _REQUEST_CONTEXT_H
#define _REQUEST_CONTEXT_H

#include "known_nodes_hashmap.h"
#include "node.h"

struct client_context {
    struct known_node_hashmap_entry * map_entry;
    struct node * node;
};

struct preamble {
    unsigned short port;
    char name[NODE_NAME_LEN];
};

void pack_context(char * output);
struct client_context extract_context(char * input, char * ip);
void destroy_context(struct client_context context);


#endif
