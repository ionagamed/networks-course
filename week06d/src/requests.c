#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>

#include "client_context.h"
#include "requests.h"
#include "logging.h"


int connect_to_node(struct node * node) {
    return connect_to_node_plain(node->ip, node->port);
}

int connect_to_node_plain(char * ip, unsigned short port) {
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

void send_request(struct node * node, char * body, size_t body_length) {
    send_request_plain(node->ip, node->port, body, body_length);
}

void send_request_plain(char * ip, unsigned short port, char * body, size_t body_length) {
    if (body_length == 0) return;

    int sock = connect_to_node_plain(ip, port);
    if (sock < 0) {
        log_debug("Could not connect to node");
        return;
    }
    send_request_sock(sock, body, body_length);
    close(sock);
}

void send_request_sock(int sock, char * body, size_t body_length) {
    if (body_length == 0) return;
    char request[sizeof(struct preamble) + body_length];
    pack_context(request);
    memcpy(&request[sizeof(struct preamble)], body, body_length);
    send(sock, request, body_length + sizeof(struct preamble), 0);
}

ssize_t send_receive(struct node * node, char * body, size_t body_length, char * response, struct client_context * context, size_t response_length) {
    return send_receive_plain(node->ip, node->port, body, body_length, response, context, response_length);
}

ssize_t send_receive_plain(char * ip, unsigned short port, char * body, size_t body_length, char * response, struct client_context * context, size_t response_length) {
    int sock = connect_to_node_plain(ip, port);
    if (sock < 0) {
        log_debug("Could not connect to node");
        return -1;
    }
    ssize_t ret = send_receive_sock(sock, ip, body, body_length, response, context, response_length);
    close(sock);
    return ret;
}
ssize_t send_receive_sock(int sock, char * ip, char * body, size_t body_length, char * response, struct client_context * context, size_t response_length) {
    char request[sizeof(struct preamble) + body_length];
    pack_context(request);
    memcpy(&request[sizeof(struct preamble)], body, body_length);
    send(sock, request, body_length + sizeof(struct preamble), 0);

    char response_with_preamble[sizeof(struct preamble) + response_length];
    ssize_t r_length = recv(sock, response_with_preamble, sizeof(struct preamble) + response_length, 0);

    if (r_length < 0) {
        return -1;
    } else {
        if (context != NULL) {
            *context = extract_context(response_with_preamble, ip);
        }
        memcpy(response, &response_with_preamble[sizeof(struct preamble)], r_length - sizeof(struct preamble));
        return r_length - sizeof(struct preamble);
    }
}
