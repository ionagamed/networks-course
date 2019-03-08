#ifndef _CLIENT_H
#define _CLIENT_H

#include "node.h"

int connect_to_bootstrap(char * bootstrap_ip, unsigned short bootstrap_port);
int get_known_nodes(int sock, struct node ** nodes, size_t * nodes_count);
int say_hello(int sock, char * name, unsigned short port, char * out_name, unsigned short * out_port);

#endif