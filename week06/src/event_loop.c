/*
 * event_loop.c
 *
 * Simple file-descriptor powered event loop
 */

#include <unistd.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>

#include "event_loop.h"

void ** event_loop_user_args;
event_loop_callback_t * event_loop_callbacks;
struct pollfd * event_loop_fds;
nfds_t event_loop_next_fd = 0;
size_t event_loop_max_fds = 0;

struct event_loop_periodic_task * event_loop_periodic_tasks;
int event_loop_next_periodic_task = 0;
size_t event_loop_max_periodic_tasks;

int * event_loop_to_remove;
int event_loop_next_to_remove = 0;

int event_loop_keep_alive = 1;

void sigterm_handler(int signum) {
    printf("\nClosing all fds\n");
    for (int i = 0; i < event_loop_next_fd; i++) {
        close(event_loop_fds[i].fd);
    }
    exit(0);
}

int event_loop_init(size_t max_fds, size_t max_periodic_tasks) {
    event_loop_max_fds = max_fds;

    event_loop_fds = calloc(max_fds, sizeof(struct pollfd));
    event_loop_user_args = calloc(max_fds, sizeof(void*));
    event_loop_callbacks = calloc(max_fds, sizeof(event_loop_callback_t));
    event_loop_periodic_tasks = calloc(max_fds, sizeof(struct event_loop_periodic_task));
    event_loop_to_remove = calloc(max_fds, sizeof(int));
    event_loop_max_periodic_tasks = max_periodic_tasks;

    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);

    if (event_loop_fds == NULL) {
        perror("calloc");
        return -1;
    }
    return 0;
}

int event_loop_add_fd(int fd, short events, event_loop_callback_t callback, void * args) {
    if (event_loop_next_fd == event_loop_max_fds) {
        return -1;
    }

    event_loop_fds[event_loop_next_fd].fd = fd;
    event_loop_fds[event_loop_next_fd].events = events;
    event_loop_fds[event_loop_next_fd].revents = 0;
    event_loop_callbacks[event_loop_next_fd] = callback;
    event_loop_user_args[event_loop_next_fd] = args;

    event_loop_next_fd++;

    return 0;
}

int event_loop_remove_fd(int fd) {
    // first find it
    int found_fd = -1;
    for (int i = 0; i < event_loop_next_fd; i++) {
        if (event_loop_fds[i].fd == fd) {
            found_fd = i;
            break;
        }
    }

    if (found_fd < 0) {
        return -1;
    }

    // then remove it
    for (int i = found_fd; i < event_loop_next_fd - 1; i++) {
        event_loop_fds[i] = event_loop_fds[i + 1];
        event_loop_callbacks[i] = event_loop_callbacks[i + 1];
        event_loop_user_args[i] = event_loop_user_args[i + 1];
    }

    event_loop_next_fd--;

    return 0;
}

void event_loop_schedule_remove_fd(int fd) {
    event_loop_to_remove[event_loop_next_to_remove++] = fd;
}

int event_loop_add_periodic_task(event_loop_periodic_callback_t callback, int interval) {
    if (event_loop_next_periodic_task == event_loop_max_periodic_tasks) {
        return -1;
    }

    event_loop_periodic_tasks[event_loop_next_periodic_task].callback = callback;
    event_loop_periodic_tasks[event_loop_next_periodic_task].interval = interval;
    event_loop_periodic_tasks[event_loop_next_periodic_task].last_done = time(NULL) - interval;
    event_loop_next_periodic_task++;

    return 0;
}

int event_loop_wait_for_events(int timeout) {
    return poll(event_loop_fds, event_loop_next_fd, timeout);
}

int event_loop_run_loop(int timeout) {
    while (event_loop_keep_alive) {
        if (event_loop_wait_for_events(timeout) >= 0) {
            for (int i = 0; i < event_loop_next_fd; i++) {
                if (event_loop_fds[i].revents != 0) {
                    event_loop_fds[i].revents = 0;
                    if (event_loop_callbacks[i](event_loop_fds[i].fd, event_loop_fds[i].revents, event_loop_user_args[i]) < 0) {
                        event_loop_schedule_remove_fd(event_loop_fds[i].fd);
                        close(event_loop_fds[i].fd);
                    }
                }
            }
        }

        for (int i = 0; i < event_loop_next_periodic_task; i++) {
            if (time(NULL) - event_loop_periodic_tasks[i].last_done > event_loop_periodic_tasks[i].interval) {
                event_loop_periodic_tasks[i].last_done = time(NULL);
                event_loop_periodic_tasks[i].callback();
            }
        }

        while (event_loop_next_to_remove != 0) {
            event_loop_remove_fd(event_loop_to_remove[event_loop_next_to_remove - 1]);
            event_loop_next_to_remove--;
        }
    }

    return 0;
}
