#include <poll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#include "known_nodes_hashmap.h"
#include "pings.h"
#include "logging.h"
#include "magic.h"
#include "event_loop.h"
#include "requests.h"
#include "globals.h"

int ping_sock(int fd, short events, struct known_node_hashmap_entry * node_entry) {
    if (events | POLLIN) {
        // TODO: refactor into normal preamble decoding
        unsigned char magic_pong_buf[sizeof(struct preamble) + 1];
        if (recv(fd, &magic_pong_buf, sizeof(struct preamble) + 1, 0) <= 0) goto ping_response_err;
        char magic_pong = magic_pong_buf[sizeof(struct preamble)];
        if (magic_pong != MAGIC_PONG) {
            log_debug("Wrong data in pong response, %u", magic_pong);
            goto ping_response_err;
        }
        node_entry->waiting_pong_since = 0;
        event_loop_schedule_remove_fd(fd);
        return 0;
    }

ping_response_err:
    perror("");
    return -1;
}

void ping_node(struct known_node_hashmap_entry * node_entry) {
    if (node_entry->waiting_pong_since == 0) {
        char magic_ping = MAGIC_PING;

        int node_sock = connect_to_node_plain(node_entry->ip, node_entry->port);
        if (node_sock < 0) {
            node_entry->waiting_pong_since = -1;
            return;
        }

        send_request_sock(node_sock, &magic_ping, 1);
        node_entry->waiting_pong_since = time(NULL);

        event_loop_add_fd(node_sock, POLLIN | POLLHUP | POLLERR, (event_loop_callback_t) ping_sock, node_entry);
    }
}

void ping_all_nodes() {
    char output_buf[BUFFER_SIZE];
    output_buf[0] = 0;
    char intermediate_buf[BUFFER_SIZE];
    intermediate_buf[0] = 0;
    sprintf(intermediate_buf, "Pings: ");
    strcat(output_buf, intermediate_buf);
    for (int i = 0; i < HASHMAP_ENTRIES; i++) {
        struct known_node_hashmap_entry * current = known_nodes[i];
        while (current != NULL) {
            if (current->waiting_pong_since == 0) {
                sprintf(intermediate_buf, " %s:ok ", current->name);
                strcat(output_buf, intermediate_buf);
            } else {
                sprintf(intermediate_buf, " %s:failed ", current->name);
                strcat(output_buf, intermediate_buf);
            }

            ping_node(current);
            current = current->next;
        }
    }
    log_info(output_buf);
}