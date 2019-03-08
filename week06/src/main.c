#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "server.h"
#include "app.h"
#include "node.h"
#include "known_nodes_hashmap.h"

char * usage =
"Usage:\n"
"   %s --listen <port> --name <node name> [--bootstrap <ip:port>] [--workers <n>]\n";

int parse_args(int argc, char ** argv, unsigned short * listen_port, char * bootstrap_addr, int * workers, char * node_name) {
    argc--; argv++;

    int got_listen_ip = 0;
    int got_name = 0;

    if (argc <= 0) return -1;

    while (argc > 0) {
        if (strcmp(argv[0], "--listen") == 0) {
            argv++;
            argc--;

            if (argc <= 0) return -1;

            *listen_port = atoi(argv[0]);
            got_listen_ip = 1;

            argc--;
            argv++;
        } else if (strcmp(argv[0], "--bootstrap") == 0) {
            argv++;
            argc--;

            if (argc <= 0) return -1;

            strcpy(bootstrap_addr, argv[0]);

            argv++;
            argc--;
        } else if (strcmp(argv[0], "--workers") == 0) {
            argv++;
            argc--;

            if (argc <= 0) return -1;
            *workers = atoi(argv[0]);

            argv++;
            argc--;
        } else if (strcmp(argv[0], "--name") == 0) {
            argv++;
            argc--;

            if (argc <= 0) return -1;

            strcpy(node_name, argv[0]);
            got_name = 1;

            argv++;
            argc--;
        } else {
            return -1;
        }
    }

    if (got_listen_ip == 1 && got_name == 1) {
        return 0;
    } else {
        return -1;
    }
}

int main(int argc, char ** argv) {
    char bootstrap_addr[NODE_IP_LEN];
    unsigned short listen_port = 0;
    int workers = 3;
    char node_name[NODE_NAME_LEN];

    if (parse_args(argc, argv, &listen_port, bootstrap_addr, &workers, node_name) != 0) {
        printf(usage, argv[0]);
        return -1;
    }

    init_known_nodes_hashmap();
    set_node_name(node_name);
    set_listen_port(listen_port);

    if (bootstrap_addr[0] != 0) {
        char * ip = strtok(bootstrap_addr, ":");
        char * port = strtok(NULL, ":");
        if (bootstrap_known_nodes(ip, (unsigned short) atoi(port), listen_port) < 0) {
            printf("Bootstrap failed\n");
            return -1;
        }
    }

    if (server_main(listen_port) < 0) {
        printf("Couldn't start server thread\n");
        return -1;
    }
}