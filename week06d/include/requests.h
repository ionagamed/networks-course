#ifndef _REQUESTS_H
#define _REQUESTS_H

#include <stdlib.h>

#include "node.h"
#include "client_context.h"

int connect_to_node(struct node * node);
int connect_to_node_plain(char * ip, unsigned short port);

void send_request(struct node * node, char * body, size_t body_length);
void send_request_plain(char * ip, unsigned short port, char * body, size_t body_length);
void send_request_sock(int sock, char * body, size_t body_length);

ssize_t send_receive(struct node * node, char * body, size_t body_length, char * response, struct client_context * context, size_t response_length);
ssize_t send_receive_plain(char * ip, unsigned short port, char * body, size_t body_length, char * response, struct client_context * context, size_t response_length);
ssize_t send_receive_sock(int sock, char * ip, char * body, size_t body_length, char * response, struct client_context * context, size_t response_length);

struct request_context {
    unsigned short port;
    char name[NODE_NAME_LEN];
};

#endif
