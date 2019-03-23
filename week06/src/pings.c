#include <poll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "known_nodes.h"
#include "pings.h"
#include "logging.h"
#include "event_loop.h"
#include "requests.h"
#include "globals.h"

int ping_sock(int fd, short events, struct known_node * known_node) {
    if (events | POLLIN) {
        char magic_pong_buf[BUFFER_SIZE];
        ssize_t nbytes = recv(fd, magic_pong_buf, sizeof(magic_pong_buf), 0);
        if (nbytes <= 0) goto ping_response_err;
        json_t * request = deserialize_json(magic_pong_buf, nbytes);
        char * response_type = json_get_property(json_get_property(request, "body"), "type")->string_value;
        if (strcmp(response_type, "pong") != 0) {
            log_debug("Wrong data in pong response: %s\n", response_type);
            return -1;
        }
        known_node->waiting_pong_since = 0;
        event_loop_schedule_remove_fd(fd);
        return 0;
    }

ping_response_err:
    perror("");
    return -1;
}

void ping_node(struct known_node * known_node) {
    if (known_node->waiting_pong_since == 0) {
        int node_sock = connect_plain(known_node->ip, known_node->port);
        if (node_sock < 0) {
            known_node->waiting_pong_since = -1;
            return;
        }

        json_t * request = pack("ping", create_json_null());
        send_request_sock(node_sock, request);
        known_node->waiting_pong_since = time(NULL);

        event_loop_add_fd(node_sock, POLLIN | POLLHUP | POLLERR, (event_loop_callback_t) ping_sock, known_node);
    }
}

struct ping_node_callback_context {
    char * intermediate_buf;
    char * output_buf;
};

void ping_node_callback(struct known_node * known_node, struct ping_node_callback_context * context) {
    if (known_node->waiting_pong_since == 0) {
        sprintf(context->intermediate_buf, " %s:ok ", known_node->name);
    } else {
        sprintf(context->intermediate_buf, " %s:failed ", known_node->name);
    }
    strcat(context->output_buf, context->intermediate_buf);
    ping_node(known_node);
}

void ping_all_nodes() {
    char output_buf[BUFFER_SIZE];
    output_buf[0] = 0;
    char intermediate_buf[BUFFER_SIZE];
    intermediate_buf[0] = 0;
    sprintf(intermediate_buf, "Pings: ");
    strcat(output_buf, intermediate_buf);

    struct ping_node_callback_context context;
    context.intermediate_buf = intermediate_buf;
    context.output_buf = output_buf;

    for_each_known_node(ping_node_callback, &context);
    log_info(output_buf);
}