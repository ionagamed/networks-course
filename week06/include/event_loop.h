#ifndef _EVENT_LOOP_H
#define _EVENT_LOOP_H

#include <time.h>
#include <poll.h>

typedef int (*event_loop_callback_t) (int, short, void*);

typedef void (*event_loop_periodic_callback_t) ();

struct event_loop_periodic_task {
    event_loop_periodic_callback_t callback;
    int interval;
    time_t last_done;
};

int event_loop_init(size_t max_fds, size_t max_nested_fds, size_t max_periodic_tasks);

int event_loop_listen_fd(int fd, short events, event_loop_callback_t callback, void * args);
void event_loop_replace_fd(int fd, short events, event_loop_callback_t callback, void * args);
void event_loop_release_fd(int fd);

int event_loop_add_periodic_task(event_loop_periodic_callback_t callback, int interval);
int event_loop_wait_for_events(int timeout);
int event_loop_run_loop(int timeout);

#endif
