#ifndef _CONTROL_H
#define _CONTROL_H

int handle_control_socket_callback(int fd, short events, void * args);
int create_control_socket(char *filename);

int control_main(char * control_socket_filename);

#endif
