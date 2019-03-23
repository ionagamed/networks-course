#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>

#include "common.h"
#include "requests.h"
#include "event_loop.h"
#include "logging.h"


int connect_to_node(struct node * node) {
    return connect_plain(node->ip, node->port);
}

int connect_plain(char * ip, unsigned short port) {
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        perror("socket");
        return -1;
    }
    struct sockaddr_in node_addr;
    node_addr.sin_family = AF_INET;

    struct hostent * host;

    if ((host = gethostbyname(ip)) == NULL) {
        perror("gethostbyname");
        close(sock);
        return -1;
    }

    node_addr.sin_addr = *(struct in_addr *) host->h_addr_list[0];
    node_addr.sin_port = htons(port);

    if (connect(sock, (struct sockaddr *) &node_addr, sizeof(struct sockaddr_in)) != 0) {
        close(sock);
        return -1;
    }

    return sock;
}

void perform_request(struct node * node, char * method, json_t * body, uint8_t flags, request_callback_t callback, void * context) {
    perform_request_plain(node->ip, node->port, method, body, keep_alive, callback, context);
}

void perform_request_plain(char * ip, uint16_t port, char * method, json_t * body, uint8_t flags, request_callback_t callback, void * context) {
    int sock = connect_plain(ip, port);
    if (sock < 0) {
        // call error callback
        return;
    }
    perform_request_sock(sock, method, body, keep_alive, callback, context);
}

struct request_context {
    char * method;
    json_t * body;
    request_sent_callback_t sent_callback;
    void * user_context;

    char * response_buf;
    size_t response_len;
    size_t max_response_len;
    request_callback_t callback;

    uint8_t flags;
    int sock;
};

int pr_recv(int sock, short events, struct request_context * context) {
    recv(sock, context->response_buf + context->response_len, context->max_response_len - context->response_len, 0);
    json_t * response_json = deserialize_json(context->response_buf, context->response_len);
    if (response_json != NULL) {  // first parsed value == whole response
        free(context->response_buf);

        response_t response;
        response.raw_data = response_json;
        response.body = json_get_property(response_json, "body");
        response.status = json_get_property(response_json, "status")->int_value;
        json_t * from = json_get_property(response_json, "from");
        response.context.name = json_get_property(from, "name")->string_value;
        response.context.listen_port = json_get_property(from, "listen_port")->int_value;

        context->callback(response, context->user_context);

        free(response_json);

        if (context->flags | RF_KEEP_LISTENING == 0) {
            event_loop_release_fd(sock);
        }
    }

    if (context->flags | RF_KEEP_ALIVE == 0) {
        close(sock);
    }

    return 0;
}

int c_finalize_recv(response_t response, struct request_context * context) {
    context->callback(response, context->user_context);
    free(context);
    return 0;
}

int c_continue_recv(struct request_context * context) {
    receive_request_sock(context->sock, (request_callback_t) c_finalize_recv, context);
    return 0;
}

void perform_request_sock(int sock, char * method, json_t * body, uint8_t flags, request_callback_t callback, void * user_context) {
    struct request_context * context = calloc(1, sizeof(struct request_context));
    context->callback = callback;
    context->user_context = user_context;
    context->sock = sock;
    context->flags = flags;
    send_request_sock(sock, method, body, flags, (request_sent_callback_t) c_continue_recv, context);
}

int pr_send(int sock, short events, struct request_context * context) {
    json_t * request = create_json_object();
    json_set_property(request, "method", create_json_string(context->method));
    json_set_property(request, "body", context->body);
    json_t * from = create_json_object();
    json_set_property(from, "name", create_json_string(node_name));
    json_set_property(from, "listen_port", create_json_integer(listen_port));
    json_set_property(request, "from", from);

    char * request_data = serialize_json(request);
    send(sock, request_data, strlen(request_data), 0);  // TODO: chunking large data

    context->sent_callback(context->user_context);

    if (context->flags | RF_KEEP_LISTENING == 0) {
        event_loop_release_fd(sock);
    }

    return 0;
}

void send_request(struct node * node, char * method, json_t * body, uint8_t flags, request_sent_callback_t callback, void * user_context) {
    send_request_plain(node->ip, node->port, method, body, flags, callback, user_context);
}

void send_request_plain(char * ip, uint16_t port, char * method, json_t * body, uint8_t flags, request_sent_callback_t callback, void * user_context) {
    int sock = connect_plain(ip, port);
    if (sock < 0) {
        return;
    }

    send_request_sock(sock, method, body, flags, callback, user_context);
}

void send_request_sock(int sock, char * method, json_t * body, uint8_t flags, request_sent_callback_t callback, void * user_context) {
    struct request_context * context = calloc(1, sizeof(struct request_context));
    context->method = method;
    context->body = body;
    context->sent_callback = callback;
    context->user_context = user_context;
    context->flags = flags;
    event_loop_listen_fd(sock, POLLOUT, (event_loop_callback_t) pr_send, context);
}

void receive_request_sock(int sock, uint8_t flags, request_callback_t callback, void * user_context) {
    struct request_context * context = calloc(1, sizeof(struct request_context));
    context->response_buf = calloc(BUFFER_SIZE, sizeof(char));
    context->response_len = 0;
    context->max_response_len = BUFFER_SIZE;
    context->user_context = user_context;
    context->callback = callback;
    context->flags = flags;
    event_loop_listen_fd(sock, POLLOUT, (event_loop_callback_t) pr_recv, context);
}