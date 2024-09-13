#ifndef LOG_H
#define LOG_H

#include "headers.h"
#include "dir.h"

typedef struct log_deque {
    int cap;
    int size;
    char **arr;
} log_deque;

log_deque *create_deq();
void fill_prev(log_deque *ldeq);
void add_command(char *comm, log_deque *l_deq);
void purge(log_deque *l_deq);
void show_log(log_deque *l_deq);
void write_on_close(log_deque *l_deq);
char *return_nth_recent(log_deque *ldeq, int n);
int execute_log(char *cmd, log_deque* ldeq);

#endif // LOG_H
