#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "client.h"
#include "node.h"
#include "magic.h"

int connect_to_bootstrap(char * bootstrap_ip, unsigned short bootstrap_port) {
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        perror("socket");
        return -1;
    }
    struct sockaddr_in bootstrap_addr;
    bootstrap_addr.sin_family = AF_INET;

    printf("%s:%d\n", bootstrap_ip, bootstrap_port);

    // hostname lookup
    struct hostent * host;
    struct in_addr ** addr_list;

    if ((host = gethostbyname(bootstrap_ip)) == NULL) {
        perror("gethostbyname");
        return -1;
    }

    bootstrap_addr.sin_addr = *(struct in_addr *) host->h_addr_list[0];
    bootstrap_addr.sin_port = htons(bootstrap_port);

    if (connect(sock, (struct sockaddr *) &bootstrap_addr, sizeof(struct sockaddr_in)) != 0) {
        perror("connect");
        return -1;
    }

    return sock;
}

int get_known_nodes(int sock, struct node ** nodes, size_t * nodes_count) {
    char buf[20480];
    buf[0] = MAGIC_GET_NODE_LIST;
    send(sock, buf, 1, 0);
    recv(sock, buf, 20480, 0);

    int cnt = *((int*) &buf[1]);
    *nodes_count = cnt;

    *nodes = calloc(cnt, sizeof(struct node));
    memcpy(*nodes, &buf[1 + sizeof(int)], sizeof(struct node) * cnt);

    return 0;
}

int say_hello(int sock, char * name, unsigned short port, char * out_name, unsigned short * out_port) {
    char buf[100];
    buf[0] = MAGIC_HELLO;
    strncpy(&buf[1], name, NODE_NAME_LEN);
    memcpy(&buf[1 + NODE_NAME_LEN], (char*)&port, sizeof(unsigned short));
    send(sock, buf, 1 + NODE_NAME_LEN + sizeof(unsigned short), 0);
    recv(sock, buf, 1 + NODE_NAME_LEN + sizeof(unsigned short), 0);
    strncpy(out_name, &buf[1], NODE_NAME_LEN);
    out_name[NODE_NAME_LEN - 1] = 0;
    memcpy(out_port, &buf[NODE_NAME_LEN + 1], sizeof(unsigned short));
    return 0;
}