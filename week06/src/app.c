#include <string.h>

#include "magic.h"
#include "app.h"
#include "client.h"
#include "node.h"
#include "known_nodes_hashmap.h"

char * node_name;
unsigned short listen_port;

void set_node_name(char * name) {
    node_name = name;
}

void set_listen_port(unsigned short port) {
    listen_port = port;
}

void on_client_request(char * input, char * output, size_t * output_len, int * keep_alive, char * ip) {
    switch (input[0]) {
        case MAGIC_HELLO:
            output[0] = MAGIC_HELLO;
            strcpy(&output[1], node_name);
            memcpy(&output[1 + NODE_NAME_LEN], (char *) &listen_port, sizeof(unsigned short));
            *output_len = 1 + NODE_NAME_LEN + sizeof(unsigned short);

            struct node client_node;
            strcpy(client_node.ip, ip);
            strncpy(client_node.name, &input[1], NODE_NAME_LEN);
            client_node.name[NODE_NAME_LEN - 1] = 0;
            memcpy(&client_node.port, &input[1 + NODE_NAME_LEN], sizeof(unsigned short));

            add_known_node(&client_node);

            *keep_alive = 1;

            break;
        case MAGIC_GET_NODE_LIST:
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
            break;
        case MAGIC_PING:
            output[0] = MAGIC_PONG;
            *output_len = 1;
            break;
        case 'P':
            dbg_print_all_known_nodes();
            break;
    }
}

int bootstrap_known_nodes(char * bootstrap_ip, unsigned short bootstrap_port, unsigned short listen_port) {
    struct node * nodes;
    size_t nodes_count;
    
    int sock = connect_to_bootstrap(bootstrap_ip, bootstrap_port);

    struct node bootstrap_node;
    unsigned short bootstrap_server_port;
    say_hello(sock, node_name, listen_port, bootstrap_node.name, &bootstrap_server_port);
    bootstrap_node.port = bootstrap_server_port;
    strcpy(bootstrap_node.ip, bootstrap_ip);
    add_known_node(&bootstrap_node);

    if (get_known_nodes(sock, &nodes, &nodes_count) != 0) {
        return -1;
    }
    for (int i = 0; i < nodes_count; i++) {
        if (strcmp(nodes[i].name, node_name) != 0) { // don't add yourself
            add_known_node(&nodes[i]);
        }
    }

    return 0;
}