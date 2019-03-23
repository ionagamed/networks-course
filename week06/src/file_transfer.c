#include <stdio.h>
#include <sys/socket.h>

#include "common.h"
#include "file_transfer.h"
#include "event_loop.h"

int handle_file_upload(int fd, short events, FILE * f) {
    char buf[BUFFER_SIZE];
    recv(fd, buf, BUFFER_SIZE, 0); // TODO: do something with it later

    int nbytes = fscanf(f, "%s", buf);
    if (nbytes > 0) {
        json_t * next_word_payload = create_json_object();
        json_set_property(next_word_payload, "next_word", create_json_string(buf));
        send_request_sock(fd, next_word_payload);
        return 0;
    } else {
        fclose(f);
        json_t * end_of_transmission = create_json_object();
        json_set_property(end_of_transmission, "end_of_transmission", create_json_boolean(1));
        send_request_sock(fd, end_of_transmission);
        return -1;
    }
}

int begin_file_upload(int client, char * path) {
    FILE * file = fopen(path, "r");
    if (!file) {
        return -1;
    }

    json_t * begin = create_json_string("begin_file_response");
    send_request_sock(client, begin);

    event_loop_replace_fd(client, POLLIN, (event_loop_callback_t) handle_file_upload, file);

    return 0;
}

int handle_file_download(int fd, short events, FILE * f) {
    char buf[BUFFER_SIZE];
    ssize_t nbytes = recv(fd, buf, BUFFER_SIZE, 0);
    if (nbytes <= 0) {
        fclose(f);
        return -1;
    }
    buf[nbytes] = 0;
    json_t * word_response = deserialize_json(buf, nbytes);
    json_t * next_word = json_get_property(json_get_property(word_response, "body"), "next_word");
    if (!next_word) {
        fclose(f);
        return -1;
    }
    fprintf(f, "%s ", next_word->string_value);
    send_request_sock(fd, pack("give_next_word", create_json_null()));
    return 0;
}

int begin_file_download(int sock, char * ip, char * filename, char * path) {
    FILE * file = fopen(path, "w");
    if (!file) {
        return -1;
    }

    json_t * request_payload = create_json_object();
    json_set_property(request_payload, "filename", create_json_string(filename));
    json_t * request = pack("get_file", request_payload);
    json_t * response = send_receive_sock(sock, ip, request, NULL);

    if (strcmp(response->string_value, "begin_file_response") == 0) {
        send_request_sock(sock, pack("begin_transfer", create_json_null()));

        event_loop_add_fd(sock, POLLIN, handle_file_download, file);
    }

    return 0;
}