#ifndef _APP_H
#define _APP_H

#include "node.h"

// common parts
void set_node_name(char * name);
void set_listen_port(unsigned short port);

// server parts
void on_client_request(char * input, char * output, size_t * output_len, int * keep_alive, char * ip);

// client parts
int bootstrap_known_nodes(char * bootstrap_ip, unsigned short bootstrap_port, unsigned short listen_port);

#endif