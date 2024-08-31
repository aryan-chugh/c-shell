#ifndef LOG_H
#define LOG_H

#include <stdbool.h>

#define BUFFER_SIZE 4096

#define RESET       "\033[0m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define WHITE       "\033[37m"
#define BOLD        "\033[1m"

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
