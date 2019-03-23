#ifndef _NODE_H
#define _NODE_H

#define NODE_NAME_LEN 20
#define NODE_IP_LEN 20

struct node {
    char name[NODE_NAME_LEN];
    char ip[NODE_IP_LEN];
    unsigned short port;
};

#endif