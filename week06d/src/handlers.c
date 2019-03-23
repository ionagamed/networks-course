#include <stdlib.h>
#include <string.h>

#include "handlers.h"
#include "client_context.h"
#include "logging.h"
#include "magic.h"
#include "globals.h"
#include "requests.h"

#define PAYLOAD_SIZE 20480

void push_node(struct known_node_hashmap_entry * entry, struct node * node) {
    char push_request[1 + sizeof(struct node)];
    push_request[0] = MAGIC_PUSH_NODE;
    memcpy(&push_request[1], node, sizeof(struct node));

    send_request_plain(entry->ip, entry->port, push_request, sizeof(push_request));
}

HANDLER(MAGIC_HELLO) {
    output[0] = MAGIC_HELLO;
    strcpy(&output[1], node_name);
    memcpy(&output[1 + NODE_NAME_LEN], (char *) &listen_port, sizeof(unsigned short));
    *output_len = 1 + NODE_NAME_LEN + sizeof(unsigned short);

    struct node client_node;
    strcpy(client_node.ip, context.node->ip);
    strncpy(client_node.name, &input[1], NODE_NAME_LEN);
    client_node.name[NODE_NAME_LEN - 1] = 0;
    memcpy(&client_node.port, &input[1 + NODE_NAME_LEN], sizeof(unsigned short));

    add_known_node(&client_node);

    *keep_alive = 1;

    for_each_known_node((known_node_callback_t) push_node, &client_node);
}

HANDLER(MAGIC_GET_NODE_LIST) {
    output[0] = MAGIC_NODE_LIST;
    int cnt = 0;
    struct node n;
    for (int i = 0; i < HASHMAP_ENTRIES; i++) {
        struct known_node_hashmap_entry * current = known_nodes[i];
        while (current != NULL) {
            strncpy(n.name, current->name, NODE_NAME_LEN);
            n.name[NODE_NAME_LEN - 1] = 0;
            strcpy(n.ip, current->ip);
            n.port = current->port;
            memcpy(&output[1 + sizeof(int) + cnt * sizeof(struct node)], (char*) &n, sizeof(struct node));
            cnt++;

            current = current->next;
        }
    }
    memcpy(&output[1], (char*) &cnt, sizeof(int));
    *output_len = 1 + sizeof(int) + cnt * sizeof(struct node);
}

HANDLER(MAGIC_PING) {
    output[0] = MAGIC_PONG;
    *output_len = 1;
}

HANDLER(MAGIC_PUSH_NODE) {
    *output_len = 0;
    struct node pushed_node;
    memcpy(&pushed_node, &input[1], sizeof(struct node));
    log_debug("Accepted push from %s:%s:%u", context.map_entry->name, context.map_entry->ip, context.map_entry->port);
    add_known_node(&pushed_node);
}


#define USE_HANDLER(magic, handler) if (input[0] == (magic)) {log_debug("invoking " #magic); handler(input, payload_output, &payload_len, keep_alive, context);}
#define USE_DEFAULT_HANDLER(magic) USE_HANDLER(magic, magic##_handler)
void on_client_request(char * input, char * output, size_t * output_len, int * keep_alive, char * ip) {
    struct client_context context = extract_context(input, ip);
    char payload_output[PAYLOAD_SIZE];
    size_t payload_len = 0;
    pack_context(output);
    input += sizeof(struct preamble);

    USE_DEFAULT_HANDLER(MAGIC_HELLO);
    USE_DEFAULT_HANDLER(MAGIC_GET_NODE_LIST);
    USE_DEFAULT_HANDLER(MAGIC_PING);
    USE_DEFAULT_HANDLER(MAGIC_PUSH_NODE);

    memcpy(&output[sizeof(struct preamble)], payload_output, payload_len);
    *output_len = payload_len + sizeof(struct preamble);

    destroy_context(context);
}
#undef USE_DEFAULT_HANDLER
#undef USE_HANDLER