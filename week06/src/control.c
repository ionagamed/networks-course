#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>

#include "common.h"
#include "control.h"
#include "known_files.h"
#include "my_files.h"
#include "event_loop.h"
#include "file_transfer.h"

#define CONTROL_HANDLER(cmd) json_t * handle_control_##cmd(json_t * payload)

CONTROL_HANDLER(add_file) {
    json_t * filename = json_get_property(payload, "filename");
    json_t * path = json_get_property(payload, "path");
    json_t * response = create_json_object();

    if (!filename || !path || filename->type != JSON_STRING || path->type != JSON_STRING) {
        json_set_property(response, "status", create_json_string("error"));
        return response;
    }

    json_set_property(response, "status", create_json_string("ok"));
    add_my_file(filename->string_value, path->string_value);

    return response;
}

CONTROL_HANDLER(my_files) {
    struct my_file * my_files[BUFFER_SIZE];
    size_t cnt;
    get_my_files(my_files, &cnt);

    json_t * response = create_json_array();

    for (int i = 0; i < cnt; i++) {
        json_t * file = create_json_object();
        json_set_property(file, "path", create_json_string(my_files[i]->path));
        json_set_property(file, "filename", create_json_string(my_files[i]->filename));
        json_array_append(response, file);
    }

    return response;
}

CONTROL_HANDLER(get_file) {
    json_t * filename = json_get_property(payload, "filename");
    json_t * path = json_get_property(payload, "path");
    json_t * response = create_json_object();
    if (!filename || !path || filename->type != JSON_STRING || path->type != JSON_STRING) {
        json_set_property(response, "status", create_json_string("error"));
        json_set_property(response, "message", create_json_string("format error"));
        return response;
    }

    struct known_file * file = find_known_file(filename->string_value);
    if (!file) {
        json_set_property(response, "status", create_json_string("error"));
        json_set_property(response, "message", create_json_string("no such file"));
        return response;
    }

    int sock = connect_to_node(file->node);
    if (sock < 0) {
        json_set_property(response, "status", create_json_string("error"));
        json_set_property(response, "message", create_json_string("connection failed"));
        return response;
    }

    begin_file_download(sock, file->node->ip, filename->string_value, path->string_value);

    return 0;
}

#undef CONTROL_HANDLER
#define USE_CONTROL_HANDLER(cmd) if (strcmp(type, #cmd) == 0) {log_debug("Invoking %s", #cmd); return handle_control_##cmd(payload);}
json_t * handle_control_socket_cmd(char * type, json_t * payload) {
    log_debug("%s", type);
    USE_CONTROL_HANDLER(add_file)
    USE_CONTROL_HANDLER(my_files)
    USE_CONTROL_HANDLER(get_file);
//    {
//        strcpy(response, "Unknown command\n");
//        *response_len = strlen(response);
//    }

    return NULL;
}
#undef USE_CONTROL_HANDLER

int handle_control_socket_data(char * cmd, char * response_data, size_t * response_len) {
    json_t * request = deserialize_json(cmd, strlen(cmd));
    if (request == NULL) {
        strcpy(response_data, "Can\'t understand you\n");
        *response_len = strlen(response_data);
        return 0;
    }
    json_t * type = json_get_property(request, "type");
    json_t * payload = json_get_property(request, "payload");
    json_t * response = handle_control_socket_cmd(type->string_value, payload);
    if (response) {
        char * response_buf = serialize_json(response);
        strcpy(response_data, response_buf);
        *response_len = strlen(response_buf);
        return 0;
    } else {
        *response_len = 0;
        return 0;
    }
}

int create_control_socket(char * filename) {
    int control_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (control_socket < 0) {
        perror("socket");
        return -1;
    }
    struct sockaddr_un socket_path;
    socket_path.sun_family = AF_UNIX;
    strcpy(socket_path.sun_path, filename);
    unlink(filename);
    if (bind(control_socket, (struct sockaddr*) &socket_path, sizeof(struct sockaddr_un)) != 0) {
        perror("bind");
        return -1;
    }
    if (listen(control_socket, MAX_CLIENTS) != 0) {
        perror("listen");
        return -1;
    }
    return control_socket;
}

int control_socket_handle_client(int fd, short events, void * args) {
    if (events | POLLIN) {
        char request[BUFFER_SIZE];
        char response[BUFFER_SIZE];
        size_t response_len = 0;
        ssize_t nbytes = recv(fd, request, sizeof(request), 0);
        if (nbytes == 0) {
            return -1;
        }
        request[nbytes] = 0;
        int r = handle_control_socket_data(request, response, &response_len);
        if (r != 0) return r;
        if (response_len != 0) {
            send(fd, response, response_len, 0);
        }
        return 0;
    }
    return -1;
}

int control_socket_handle_new_connection(int fd, short events, void * args) {
    int new_client = accept(fd, NULL, NULL);
    log_debug("Accepted new control connection");
    event_loop_add_fd(new_client, POLLIN, control_socket_handle_client, NULL);
    return 0;
}

int control_main(char * control_socket_filename) {
    int control_socket = create_control_socket(control_socket_filename);
    if (control_socket < 0) return -1;

    event_loop_add_fd(control_socket, POLLIN, control_socket_handle_new_connection, NULL);

    return 0;
}
