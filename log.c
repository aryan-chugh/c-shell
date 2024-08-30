#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

log_deque *create_deq() {
    log_deque *ldeq = (log_deque *) malloc(sizeof(log_deque));
    ldeq->cap = 15;
    ldeq->arr = (char **) malloc(sizeof(char *) * (ldeq->cap));
    for (int i = 0; i < ldeq->cap; i++) {
        ldeq->arr[i] = (char *) malloc(sizeof(char) * BUFFER_SIZE);
    }
    ldeq->size = 0; // Initialize size
    return ldeq;
}

void fill_prev(log_deque *ldeq) {
    int flags = O_RDONLY;
    int fd = open("log.txt", flags);

    if (fd == -1) {
        ldeq->size = 0;
        return;
    } else {
        int fd1 = dup(STDIN_FILENO);
        if (dup2(fd, STDIN_FILENO) == -1) {
            perror("dup2 error");
        } else {
            int count = 0;
            char *s = (char *) malloc(sizeof(char) * BUFFER_SIZE);

            int lc;
            scanf("%d", &lc);
            char d;
            scanf("%c", &d);

            for (int i = 0; i < lc; i++) {
                scanf("%[^\n]%*c", s);
                strcpy(ldeq->arr[count++], s);
            }

            dup2(fd1, STDIN_FILENO);
            ldeq->size = count;
        }
    }
    close(fd);
}

// adds the command only checking if the most recent is not the same.
// checking for the log in the command will be done in the main method.
void add_command(char *comm, log_deque *l_deq) {
    if (strcmp(comm, l_deq->arr[0]) == 0) {
        return;
    }

    int start = l_deq->size == l_deq->cap ? (l_deq->cap) - 1 : l_deq->size;
    for (int i = start; i > 0; i--) {
        strcpy(l_deq->arr[i], l_deq->arr[i - 1]);
    }
    if (l_deq->size < l_deq->cap) {
        (l_deq->size)++;
    }

    strcpy(l_deq->arr[0], comm);
}

void purge(log_deque *l_deq) {
    l_deq->size = 0;
}

void show_log(log_deque *l_deq) {
    for (int i = (l_deq->size) - 1; i >= 0; i--) {
        printf("%s\n", l_deq->arr[i]);
        fflush(stdout);
    }
}

void write_on_close(log_deque *l_deq) {
    int flags = O_CREAT | O_TRUNC | O_WRONLY;
    mode_t mode = 0666;
    int fd = open("log.txt", flags, mode);
    if (fd == -1) {
        perror("open error");
        return;
    }

    int fd1 = dup(STDOUT_FILENO);
    int fd2 = dup2(fd, STDOUT_FILENO);

    printf("%d\n", l_deq->size);
    
    show_log(l_deq);
    dup2(fd1, STDOUT_FILENO);

    close(fd);
}

char *return_nth_recent(log_deque *ldeq, int n) {
    n--;
    if (n >= ldeq->size) {
        // exceeds history size : error
        printf(RED "Error: Command not available!\n" WHITE);
        return NULL; // handle it as it is not a valid command.
    } else {
        return ldeq->arr[n];
    }
}

int execute_log(char *cmd, log_deque* ldeq) {
    if (strlen(cmd) == 0) {
        show_log(ldeq);
        return 0; // success
    } else {
        if (strcmp("purge", cmd) == 0) {
            purge(ldeq);
            return 0; // success
        } else if (strncmp("execute", cmd, 7) == 0) {
            int id;
            char *tok = strtok(cmd, " ");
            tok = strtok(NULL, " ");

            if(tok == NULL) {
                printf(RED "Error: Index not specified!\n" WHITE);
                return -1;
            }

            if (id = atoi(tok)) {
                return id; // success
            } else {
                printf(RED "Error: Invalid command!\n" WHITE);
                return -1; // error
            }
        } else {
            // error
            return -1;
        }
    }
}
