#ifndef _HANDLERS_H
#define _HANDLERS_H

#define HANDLER(name) void name##_handler(char * input, char * output, size_t * output_len, int * keep_alive, struct client_context context)

void on_client_request(char * input, char * output, size_t * output_len, int * keep_alive, char * ip);

#endif
