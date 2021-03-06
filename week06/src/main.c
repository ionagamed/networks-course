#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "server.h"
#include "bootstrap.h"
#include "node.h"
#include "my_files.h"
#include "known_files.h"
#include "known_nodes.h"
#include "logging.h"
#include "control.h"
#include "globals.h"
#include "event_loop.h"
#include "requests.h"

#include "include/ds/json.h"

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
"   -p|--ping <secs>          ping once every <secs> seconds\n"
"   -s|--control <filename>   control socket filename (default /var/run/node.sock)\n"
"   --files <directory>       where to load files from\n";

#define ADVANCE_ARGS() argc--; argv++;
#define ADVANCE_ARGS_E() ADVANCE_ARGS(); if (argc <= 0) return -1;
int parse_args(int argc, char ** argv, char * bootstrap_addr, char * log_level, char * control_socket_filename, char * load_files_from) {
    argc--; argv++;

    int got_listen_ip = 0;
    int got_name = 0;
    strcpy(log_level, "info");
    strcpy(control_socket_filename, "/var/run/node.sock");

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
        } else if (strcmp(argv[0], "-s") == 0 || strcmp(argv[0], "--control") == 0) {
            ADVANCE_ARGS_E();
            strcpy(control_socket_filename, argv[0]);
            ADVANCE_ARGS();
        } else if (strcmp(argv[0], "--files") == 0) {
            ADVANCE_ARGS_E();
            strcpy(load_files_from, argv[0]);
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
    init_refcounter();

    char bootstrap_addr[NODE_IP_LEN];
    char _log_level[10];
    char control_socket_filename[104];
    char load_files_from[1000];
    load_files_from[0] = 0;

    if (parse_args(argc, argv, bootstrap_addr, _log_level, control_socket_filename, load_files_from) != 0) {
        printf(usage, argv[0]);
        return -1;
    }

    if (set_log_level(_log_level) != 0) {
        printf(usage, argv[0]);
        return -1;
    }

    init_my_files_hashmap();
    init_known_files_hashmap();
    init_known_nodes_hashmap();

    if (strlen(load_files_from) != 0) {
        load_my_files_from(load_files_from);
    }

    if (bootstrap_addr[0] != 0) {
        char * ip = strtok(bootstrap_addr, ":");
        char * port = strtok(NULL, ":");
        if (bootstrap_known_nodes(ip, (unsigned short) atoi(port), listen_port) < 0) {
            log_info("Bootstrap failed");
            return -1;
        }
    }

    event_loop_init(MAX_CLIENTS + 1, MAX_PERIODIC_TASKS);

    if (server_main(listen_port) < 0) {
        log_info("Couldn't start server");
        return -1;
    }

    if (control_main(control_socket_filename) < 0) {
        log_info("Couldn't start control");
        return -1;
    }

    event_loop_run_loop(EVENT_LOOP_TIMEOUT);
}