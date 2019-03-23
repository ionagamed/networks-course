#include <string.h>
#include <stdlib.h>

#include "client_context.h"
#include "globals.h"

struct client_context extract_context(char * input, char * ip) {
    struct client_context context;
    context.node = malloc(sizeof(struct node));
    strcpy(context.node->ip, ip);

    struct preamble preamble;
    memcpy(&preamble, input, sizeof(struct preamble));

    strcpy(context.node->name, preamble.name);
    context.node->port = preamble.port;

    context.map_entry = look_up_known_node(context.node->ip, context.node->port);

    return context;
}

void destroy_context(struct client_context context) {
    free(context.node);
}

void pack_context(char * output) {
    struct preamble preamble;
    strcpy(preamble.name, node_name);
    preamble.port = listen_port;
    memcpy(output, &preamble, sizeof(struct preamble));
}