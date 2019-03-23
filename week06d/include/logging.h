#ifndef _LOGGING_H
#define _LOGGING_H

int set_log_level(char * log_level);
void log_level(int level, char * filename, int line, char * fmt, ...);

enum {LOG_INFO, LOG_DEBUG};

#define log_debug(...) log_level(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) log_level(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)

#endif
