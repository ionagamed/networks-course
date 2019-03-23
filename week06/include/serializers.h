#ifndef _SERIALIZERS_H
#define _SERIALIZERS_H

#include "common.h"

json_t * serialize_node(struct node * node);
struct node * deserialize_node(json_t * data);

char * node_spec(struct node * node);

#endif
