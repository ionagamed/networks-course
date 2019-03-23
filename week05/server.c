//Taken from Abhishek Sagar

// gcc -lpthread server.c -o server
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include "common.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

struct worker_context {
    int thread_id;
    int master_socket;

    char data_buffer[10240];

    test_struct_t * client_data;
    result_struct_t result;

    struct sockaddr_in client_addr;
    socklen_t client_addr_len;

    int bytes;
};

void * worker(struct worker_context * context) {
    context->client_addr_len = sizeof(struct sockaddr_in);

    while (1) {
        printf("[%d] Trying to receive data\n", context->thread_id);
        context->bytes = recvfrom(
            context->master_socket,
            context->data_buffer,
            sizeof(context->data_buffer),
            0,
            (struct sockaddr*) &context->client_addr,
            &context->client_addr_len
        );

        printf(
            "[%d] Received %d bytes from client %s:%u\n", 
            context->thread_id, 
            context->bytes,
            inet_ntoa(context->client_addr.sin_addr),
            ntohs(context->client_addr.sin_port)
        );

        context->client_data = (test_struct_t *) context->data_buffer;

        printf(
            "[%d] Struct contents: (%s, %u, %s)\n",
            context->thread_id,
            context->client_data->name,
            context->client_data->age,
            context->client_data->group
        );

        printf("[%d] Waiting 10 seconds before reply...\n", context->thread_id);
        sleep(10);
        printf("[%d] Waking up, and sending data\n", context->thread_id);

        sprintf(
            context->result.message,
            "Hello %s, who is %u years old, and studies in %s group\n", 
            context->client_data->name, 
            context->client_data->age, 
            context->client_data->group
        );

        context->bytes = sendto(
            context->master_socket,
            (void *) &context->result,
            sizeof(context->result),
            0,
            (struct sockaddr *) &context->client_addr,
            context->client_addr_len
        );
        if (context->bytes < 0) {
            perror("sendto: ");
            exit(-1);
        }

        printf("[%d] Sent %u bytes to client\n", context->thread_id, context->bytes);
    }
}

void setup_threads(int bind_port, int max_workers) {
    int master_socket = 0;
    int sent_recv_bytes = 0;

    socklen_t addr_len;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    if ((master_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("socket create: ");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(bind_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    addr_len = sizeof(struct sockaddr);
    if (bind(master_socket, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1) {
        printf("socket bind failed\n");
        return;
    }

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(master_socket, (struct sockaddr *)&sin, &len) == -1)
        perror("getsockname");
    else
        printf("port number %d\n", ntohs(sin.sin_port));

    printf("Starting %d worker threads\n", max_workers);

    struct worker_context * contexts = malloc(max_workers * sizeof(struct worker_context));
    pthread_t *threads = malloc(max_workers * sizeof(pthread_t));
    if (!threads) {
        perror("malloc threads: ");
        exit(-1);
    }

    for (int i = 0; i < max_workers; i++) {
        contexts[i].thread_id = i;
        contexts[i].master_socket = master_socket;
        pthread_create(&threads[i], 0, worker, (void*) &contexts[i]);
    }

    printf("Joining all threads\n");
    for (int i = 0; i < max_workers; i++) {
        pthread_join(threads[i], NULL);
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage:\n    %s <server bind port> <worker threads>", argv[0]);
        return 2;
    }

    setup_threads(atoi(argv[1]), atoi(argv[2]));
    return 0;
}
