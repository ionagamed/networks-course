#ifndef _REQUESTS_H
#define _REQUESTS_H

#include <stdlib.h>

#include "common.h"
#include "node.h"

enum {
    R_OK,
    R_NO_CONTENT,
    R_ERR,
};

enum {
    RF_KEEP_ALIVE = 1,
    RF_KEEP_LISTENING = 2
};

typedef struct {
    char * name;
    uint16_t listen_port;
} response_context_t;

typedef struct {
    int status;
    json_t * body;
    response_context_t context;

    json_t * raw_data;
} response_t;

typedef int (*request_sent_callback_t) (void * context);
typedef int (*request_callback_t) (response_t response, void * context);

void perform_request(struct node * node, char * method, json_t * body, uint8_t flags, request_callback_t callback, void * context);
void perform_request_plain(char * ip, uint16_t port, char * method, json_t * body, uint8_t flags, request_callback_t callback, void * context);
void perform_request_sock(int sock, char * method, json_t * body, uint8_t flags, request_callback_t callback, void * context);

void send_request(struct node * node, char * method, json_t * body, uint8_t flags, request_sent_callback_t callback, void * context);
void send_request_plain(char * ip, uint16_t port, char * method, json_t * body, uint8_t flags, request_sent_callback_t callback, void * context);
void send_request_sock(int sock, char * method, json_t * body, uint8_t flags, request_sent_callback_t callback, void * context);

void receive_request_sock(int sock, uint8_t flags, request_callback_t callback, void * context)

int connect_to_node(struct node * node);
int connect_plain(char * ip, unsigned short port);


#endif
