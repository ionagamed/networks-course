#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

#include "logging.h"

char * level_names[] = {
        "info", "debug"
};

int log_max_level;

int set_log_level(char * log_level) {
    for (int i = 0; i < sizeof(level_names); i++) {
        if (strcmp(log_level, level_names[i]) == 0) {
            log_max_level = i;
            return 0;
        }
    }
    return -1;
}

void log_level(int level, char * filename, int line, char * fmt, ...) {
    if (level > log_max_level) return;

    time_t t = time(NULL);
    struct tm *lt = localtime(&t);

    va_list args;
    char buf[16];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", lt)] = '\0';
    fprintf(stderr, "%s %-5s %s:%d - ", buf, level_names[level], filename, line);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
}