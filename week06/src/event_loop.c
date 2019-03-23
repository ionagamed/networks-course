#include <unistd.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>

#include "event_loop.h"
#include "logging.h"


void *** event_loop_user_args;
event_loop_callback_t ** event_loop_callbacks;
struct pollfd * event_loop_pollfds;
int * event_loop_fds;
short ** event_loop_events;
nfds_t event_loop_next_fd = 0;
size_t event_loop_max_fds = 0;

struct event_loop_periodic_task * event_loop_periodic_tasks;
size_t event_loop_next_periodic_task = 0;
size_t event_loop_max_periodic_tasks = 0;

size_t * event_loop_stack_tops;
size_t event_loop_max_stack_len;

int * event_loop_to_release;
int event_loop_next_to_release;

int event_loop_keep_alive;

void sigterm_handler(int signum) {
    log_debug("Closing all fds");
    for (int i = 0; i < event_loop_next_fd; i++) {
        close(event_loop_fds[i]);
    }
    exit(0);
}

int event_loop_init(size_t max_fds, size_t max_fd_overrides, size_t max_periodic_tasks) {
    event_loop_max_fds = max_fds;

    event_loop_fds = calloc(max_fds, sizeof(int));
    event_loop_pollfds = calloc(max_fds, sizeof(struct pollfd));
    event_loop_events = calloc(max_fds, sizeof(short));
    event_loop_user_args = calloc(max_fds, sizeof(void **));
    event_loop_callbacks = calloc(max_fds, sizeof(event_loop_callback_t *));

    for (int i = 0; i < max_fds; i++) {
        event_loop_user_args = calloc(max_fd_overrides, sizeof(struct pollfd *));
        event_loop_callbacks = calloc(max_fd_overrides, sizeof(event_loop_callback_t));
    }

    event_loop_periodic_tasks = calloc(max_fds, sizeof(struct event_loop_periodic_task));
    event_loop_to_release = calloc(max_fds, sizeof(int));
    event_loop_max_periodic_tasks = max_periodic_tasks;

    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);

    if (event_loop_fds == NULL) {
        perror("calloc");
        return -1;
    }
    return 0;
}

ssize_t event_loop_find_fd(int fd) {
    for (size_t i = 0; i < event_loop_next_fd; i++) {
        if (event_loop_fds[i] == fd) {
            return i;
        }
    }
    return -1;
}

int event_loop_listen_fd(int fd, short events, event_loop_callback_t callback, void * args) {
    if (event_loop_next_fd == event_loop_max_fds) {
        return -1;
    }

    ssize_t found_fd = event_loop_find_fd(fd);
    if (found_fd < 0) {
        found_fd = event_loop_next_fd;
        event_loop_next_fd++;
    }

    ssize_t found_top = event_loop_stack_tops[found_fd];
    if (found_top == event_loop_max_stack_len) {
        return -1;
    }

    event_loop_fds[found_fd] = fd;
    event_loop_events[found_fd][found_top] = events;
    event_loop_callbacks[found_fd][found_top] = callback;
    event_loop_user_args[found_fd][found_top] = args;

    return 0;
}

void event_loop_actual_release_fd(int fd) {
    ssize_t found_fd = event_loop_find_fd(fd);
    if (found_fd < 0) return;

    ssize_t found_top = event_loop_stack_tops[found_fd];
    if (found_top > 1) {
        event_loop_stack_tops[found_fd]--;
    } else {
        for (size_t i = (size_t) found_fd; i < event_loop_next_fd - 1; i++) {
            event_loop_fds[i] = event_loop_fds[i + 1];
            for (size_t j = 0; j < event_loop_stack_tops[i]; j++) {
                event_loop_events[i][j] = event_loop_events[i + 1][j];
                event_loop_callbacks[i][j] = event_loop_callbacks[i + 1][j];
                event_loop_user_args[i][j] = event_loop_user_args[i + 1][j];
            }
            event_loop_stack_tops[i] = event_loop_stack_tops[i + 1];
        }
        event_loop_next_fd--;
    }
}

void event_loop_replace_fd(int fd, short events, event_loop_callback_t callback, void * args) {
    event_loop_actual_release_fd(fd);
    event_loop_listen_fd(fd, events, callback, args);
}

void event_loop_release_fd(int fd) {
    event_loop_to_release[event_loop_next_to_release] = fd;
    event_loop_next_to_release++;
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
    for (size_t i = 0; i < event_loop_next_fd; i++) {
        size_t j = event_loop_stack_tops[i];

        event_loop_pollfds[i].fd = event_loop_fds[i];
        event_loop_pollfds[i].events = event_loop_events[i][j];
        event_loop_pollfds[i].revents = 0;
    }

    return poll(event_loop_pollfds, event_loop_next_fd, timeout);
}

int event_loop_run_loop(int timeout) {
    while (event_loop_keep_alive) {
        if (event_loop_wait_for_events(timeout) >= 0) {
            for (int i = 0; i < event_loop_next_fd; i++) {
                size_t j = event_loop_stack_tops[i] - 1;

                if (event_loop_pollfds[i].revents != 0) {
                    int result = event_loop_callbacks[i][j](event_loop_fds[i], event_loop_pollfds[i].revents, event_loop_user_args[i][j]);
                    if (result < 0) {
                        event_loop_release_fd(event_loop_fds[i]);
                        log_debug("Dropping fd %d", event_loop_fds[i]);
                        close(event_loop_fds[i]);
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

        while (event_loop_next_to_release != 0) {
            event_loop_actual_release_fd(event_loop_to_release[event_loop_next_to_release - 1]);
            event_loop_next_to_release--;
        }
    }

    return 0;
}
