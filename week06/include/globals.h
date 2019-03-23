#ifndef _GLOBALS_H
#define _GLOBALS_H

#include "node.h"

#define BUFFER_SIZE 20480

#define MAX_CLIENTS 1024
#define MAX_PERIODIC_TASKS 10

#define EVENT_LOOP_TIMEOUT 1000

#define HASHMAP_ENTRIES 63

#define KNOWN_FILENAME_LEN 100
#define MY_PATH_LEN 500

extern unsigned short listen_port;
extern char node_name[NODE_NAME_LEN];
extern int ping_interval;

#endif
