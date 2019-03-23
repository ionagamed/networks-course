#include <stdlib.h>
#include <string.h>

#include "known_nodes_hashmap.h"
#include "bootstrap.h"
#include "requests.h"
#include "globals.h"
#include "magic.h"

int get_known_nodes(int sock, char * ip, struct node ** nodes, size_t * nodes_count) {
    char buf[20480];
    buf[0] = MAGIC_GET_NODE_LIST;

    send_receive_sock(sock, ip, buf, 1, buf, NULL, 20480);

    int cnt = *((int*) &buf[1]);
    *nodes_count = cnt;

    *nodes = calloc(cnt, sizeof(struct node));
    memcpy(*nodes, &buf[1 + sizeof(int)], sizeof(struct node) * cnt);

    return 0;
}


int say_hello(int sock, char * ip, char * name, unsigned short port, char * out_name, unsigned short * out_port) {
    char buf[100];
    buf[0] = MAGIC_HELLO;
    strncpy(&buf[1], name, NODE_NAME_LEN);
    memcpy(&buf[1 + NODE_NAME_LEN], (char*)&port, sizeof(unsigned short));
    send_receive_sock(sock, ip, buf, 1 + NODE_NAME_LEN + sizeof(unsigned short), buf, NULL, 1 + NODE_NAME_LEN + sizeof(unsigned short));
    strncpy(out_name, &buf[1], NODE_NAME_LEN);
    out_name[NODE_NAME_LEN - 1] = 0;
    memcpy(out_port, &buf[NODE_NAME_LEN + 1], sizeof(unsigned short));
    return 0;
}

int bootstrap_known_nodes(char * bootstrap_ip, unsigned short bootstrap_port, unsigned short listen_port) {
    struct node * nodes;
    size_t nodes_count;

    int sock = connect_to_node_plain(bootstrap_ip, bootstrap_port);

    struct node bootstrap_node;
    unsigned short bootstrap_server_port;
    say_hello(sock, bootstrap_ip, node_name, listen_port, bootstrap_node.name, &bootstrap_server_port);
    bootstrap_node.port = bootstrap_server_port;
    strcpy(bootstrap_node.ip, bootstrap_ip);
    add_known_node(&bootstrap_node);

    if (get_known_nodes(sock, bootstrap_ip, &nodes, &nodes_count) != 0) {
        return -1;
    }
    for (int i = 0; i < nodes_count; i++) {
        if (strcmp(nodes[i].name, node_name) != 0) { // don't add yourself
            add_known_node(&nodes[i]);
        }
    }

    return 0;
}