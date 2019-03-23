#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "server.h"
#include "bootstrap.h"
#include "node.h"
#include "known_nodes_hashmap.h"
#include "logging.h"

char node_name[NODE_NAME_LEN];
unsigned short listen_port;
int ping_interval = 3;

char * usage =
"Usage:\n"
"   %s --listen <port> --name <node name> [options]\n"
"Options:\n"
"   --listen  <name>          listen port\n"
"   --name <node name>        set node name\n"
"   --bootstrap <ip:port>     bootstrap node list from <ip:port> node\n"
"   -l <debug | info>         set logging level (default info)\n"
"   -p|--ping <secs>          ping once every <secs> seconds\n";

#define ADVANCE_ARGS() argc--; argv++;
#define ADVANCE_ARGS_E() ADVANCE_ARGS(); if (argc <= 0) return -1;
int parse_args(int argc, char ** argv, char * bootstrap_addr, char * log_level) {
    argc--; argv++;

    int got_listen_ip = 0;
    int got_name = 0;
    strcpy(log_level, "info");

    if (argc <= 0) return -1;

    while (argc > 0) {
        if (strcmp(argv[0], "--listen") == 0) {
            ADVANCE_ARGS_E();
            listen_port = atoi(argv[0]);
            got_listen_ip = 1;
            ADVANCE_ARGS();
        } else if (strcmp(argv[0], "--bootstrap") == 0) {
            ADVANCE_ARGS_E();
            strcpy(bootstrap_addr, argv[0]);
            ADVANCE_ARGS();
        } else if (strcmp(argv[0], "-l") == 0) {
            ADVANCE_ARGS_E();
            strcpy(log_level, argv[0]);
            ADVANCE_ARGS();
        } else if (strcmp(argv[0], "--name") == 0) {
            ADVANCE_ARGS_E();
            strcpy(node_name, argv[0]);
            got_name = 1;
            ADVANCE_ARGS();
        } else if (strcmp(argv[0], "-p") == 0 || strcmp(argv[0], "--ping") == 0) {
            ADVANCE_ARGS_E();
            ping_interval = atoi(argv[0]);
            ADVANCE_ARGS();
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
    char _log_level[10];

    if (parse_args(argc, argv, bootstrap_addr, _log_level) != 0) {
        printf(usage, argv[0]);
        return -1;
    }

    if (set_log_level(_log_level) != 0) {
        printf(usage, argv[0]);
        return -1;
    }

    init_known_nodes_hashmap();

    if (bootstrap_addr[0] != 0) {
        char * ip = strtok(bootstrap_addr, ":");
        char * port = strtok(NULL, ":");
        if (bootstrap_known_nodes(ip, (unsigned short) atoi(port), listen_port) < 0) {
            log_info("Bootstrap failed");
            return -1;
        }
    }

    if (server_main(listen_port) < 0) {
        log_info("Couldn't start server thread");
        return -1;
    }
}