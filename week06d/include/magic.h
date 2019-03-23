#ifndef _MAGIC_H
#define _MAGIC_H

enum {
    MAGIC_PING = 0x01,
    MAGIC_PONG = 0x02,

    MAGIC_HELLO = 'H',

    MAGIC_GET_NODE_LIST = 'G',
    MAGIC_NODE_LIST = 'N',

    MAGIC_PUSH_NODE = 'P'
};

#endif