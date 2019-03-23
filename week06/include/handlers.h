#ifndef _HANDLERS_H
#define _HANDLERS_H

#include "common.h"

#define HANDLER(name) json_t * name##_handler(json_t * payload, int * keep_alive, json_t * from)

json_t * on_client_request(json_t * request, int * keep_alive);

#endif
