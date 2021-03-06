#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "event_loop.h"
#include "server.h"
#include "node.h"
#include "known_nodes.h"
#include "handlers.h"
#include "pings.h"
#include "common.h"

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
    int keep_alive = 0;

    ssize_t nbytes = recv(socket, input, BUFFER_SIZE, 0);

    json_t * request = deserialize_json(input, (uint32_t) nbytes);
    json_t * request_from = json_get_property(request, "from");
    json_set_property(request_from, "ip", create_json_string(ip));
    json_set_property(request_from, "socket", create_json_integer(socket));

    json_t * response = on_client_request(request, &keep_alive);

    if (response != NULL) {
        char * output = serialize_json(response);
        send(socket, output, strlen(output), 0);
        json_destroy(response);
    }

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

        event_loop_listen_fd(client_socket, POLLIN | POLLHUP | POLLERR, (event_loop_callback_t) handle_client_data, inet_ntoa(client_addr.sin_addr));
    }

    return 0;
}

int server_main(unsigned short listen_port) {
    int master_socket = create_master_socket(listen_port);
    if (master_socket < 0) return -1;

    event_loop_listen_fd(master_socket, POLLIN | POLLHUP | POLLERR, handle_new_connection, NULL);
    event_loop_add_periodic_task(ping_all_nodes, ping_interval);

    return 0;
}
