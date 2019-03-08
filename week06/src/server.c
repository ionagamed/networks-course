#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "event_loop.h"
#include "server.h"
#include "app.h"
#include "node.h"
#include "magic.h"
#include "known_nodes_hashmap.h"

#define MAX_CLIENTS 1024
#define MAX_PERIODIC_TASKS 10
#define BUFFER_SIZE 2048

#define PING_TIMEOUT 3
#define PING_INTERVAL 1
#define EVENT_LOOP_TIMEOUT 1000

int create_master_socket(unsigned short listen_port) {
    int true = 1;

    int master_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (master_socket < 0) {
        perror("socket");
        return -1;
    }

    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int)) != 0) {
        perror("setsockopt");
        return -1;
    }

    struct sockaddr_in listen_addr;
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = htons(listen_port);

    if (bind(master_socket, (struct sockaddr *) &listen_addr, sizeof(struct sockaddr_in)) < 0) {
        perror("bind");
        return -1;
    }

    if (listen(master_socket, MAX_CLIENTS) < 0) {
        perror("listen");
        return -1;
    }

    return master_socket;
}

int handle_client_data(int socket, short events, char * ip) {
    char input[BUFFER_SIZE];
    char output[BUFFER_SIZE];
    size_t output_len = 0;
    int keep_alive = 0;

    recv(socket, input, BUFFER_SIZE, 0);
    on_client_request(input, output, &output_len, &keep_alive, ip);
    send(socket, output, output_len, 0);

    if (keep_alive == 0) {
        return -1;
    } else {
        return 0;
    }
}

int handle_new_connection(int socket, short events, void * args) {
    if (events | POLLIN) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(struct sockaddr_in);

        int client_socket = accept(socket, (struct sockaddr *) &client_addr, &client_addr_len);

        event_loop_add_fd(client_socket, POLLIN | POLLHUP | POLLERR, (event_loop_callback_t) handle_client_data, inet_ntoa(client_addr.sin_addr));
    }

    return 0;
}

int ping_connect_to_node(struct known_node_hashmap_entry * node_entry) {
    int node_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (node_sock < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(node_entry->port);
    inet_aton(node_entry->ip, &client_addr.sin_addr);

    if (connect(node_sock, (struct sockaddr *) &client_addr, sizeof(struct sockaddr_in)) < 0) {
        perror("connect");
        close(node_sock);
        return -1;
    }

    return node_sock;
}


int ping_sock(int fd, short events, struct known_node_hashmap_entry * node_entry) {
    if (events | POLLIN) {
        unsigned char magic_pong = 0;
        if (recv(fd, &magic_pong, 1, 0) <= 0) goto ping_response_err;
        if (magic_pong != MAGIC_PONG) {
            printf("Wrong data, bitch, %u\n", magic_pong);
            goto ping_response_err;
        }
        node_entry->waiting_pong_since = 0;
//        printf("%s:%s:%u is alive\n", node_entry->name, node_entry->ip, node_entry->port);
        event_loop_schedule_remove_fd(fd);
        return 0;
    }

ping_response_err:
    perror("");
    // remove hashmap entry
    return -1;
}


void ping_node(struct known_node_hashmap_entry * node_entry) {
    if (node_entry->waiting_pong_since == 0) {
        unsigned char magic_ping = MAGIC_PING;

        int node_sock = ping_connect_to_node(node_entry);
        if (node_sock < 0) {
            node_entry->waiting_pong_since = -1;
            return;
        }

        send(node_sock, &magic_ping, 1, 0);
        node_entry->waiting_pong_since = time(NULL);

        event_loop_add_fd(node_sock, POLLIN | POLLHUP | POLLERR, (event_loop_callback_t) ping_sock, node_entry);
    } else {
        if (time(NULL) - node_entry->waiting_pong_since > PING_TIMEOUT) {
            // somehow remove the node here
//            printf("%s:%s:%u is not alive (timout %d)\n", node_entry->name, node_entry->ip, node_entry->port, time(NULL) - node_entry->waiting_pong_since);
//            node_entry->waiting_pong_since = 0;
        }
    }
}

void ping() {
    printf("Pings: \n");
    for (int i = 0; i < HASHMAP_ENTRIES; i++) {
        struct known_node_hashmap_entry * current = known_nodes[i];
        while (current != NULL) {
            printf("%s: ", current->name);
            if (current->waiting_pong_since == 0) {
                printf("ok\n");
            } else {
                printf("failed\n");
            }

            ping_node(current);
            current = current->next;
        }
    }
}

int server_main(unsigned short listen_port) {
    int master_socket = create_master_socket(listen_port);
    if (master_socket < 0) return -1;

    event_loop_init(MAX_CLIENTS + 1, MAX_PERIODIC_TASKS);
    event_loop_add_fd(master_socket, POLLIN | POLLHUP | POLLERR, handle_new_connection, NULL);
    event_loop_add_periodic_task(ping, PING_INTERVAL);
    event_loop_run_loop(EVENT_LOOP_TIMEOUT);

    return 0;
}
