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
    if(ldeq == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        exit(EXIT_FAILURE);
    }

    ldeq->cap = 15;
    ldeq->arr = (char **) malloc(sizeof(char *) * (ldeq->cap));
    if(ldeq -> arr == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < ldeq->cap; i++) {
        ldeq->arr[i] = (char *) malloc(sizeof(char) * 4096);
        if(ldeq->arr[i] == NULL) {
            fprintf(stderr, RED);
            perror("malloc");
            fprintf(stderr, WHITE);
            exit(EXIT_FAILURE);
        }
    }
    ldeq->size = 0; // Initialize size
    return ldeq;
}

void fill_prev(log_deque *ldeq) {
    int flags = O_RDONLY;
    int fd = open("log.txt", flags);

    if (fd == -1) {
        ldeq->size = 0;
    } else {
        int fd1 = dup(STDIN_FILENO);
        if(fd1 == -1) {
            fprintf(stderr, RED);
            perror("Error: file stream couldn't be duplicated");
            fprintf(stderr, WHITE);
            exit(EXIT_FAILURE);
        }

        if (dup2(fd, STDIN_FILENO) == -1) {
            fprintf(stderr, RED);
            perror("Error: file stream couldn't be duplicated");
            fprintf(stderr, WHITE);
            exit(EXIT_FAILURE);
        } else {
            int count = 0;
            char *s = (char *) malloc(sizeof(char) * 4096);
            if(s == NULL) {
                fprintf(stderr, RED);
                perror("malloc");
                fprintf(stderr, WHITE);
                exit(EXIT_FAILURE);
            }
            int lc;
            scanf("%d", &lc);
            char d;
            scanf("%c", &d);

            for (int i = 0; i < lc; i++) {
                scanf("%[^\n]%*c", s);
                strcpy(ldeq->arr[count++], s);
            }

            if (dup2(fd1, STDIN_FILENO) == -1) {
                fprintf(stderr, RED);
                perror("Error: file stream couldn't be duplicated");
                fprintf(stderr, WHITE);
                exit(EXIT_FAILURE);
            } 
            ldeq->size = count;
            free(s);
        }
    }
    close(fd);

    char *tmp = (char *) malloc(sizeof(char) * 4096);
    if(tmp == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < (ldeq -> size) / 2; i ++) {
        int ind1 = i;
        int ind2 = (ldeq -> size) - 1 - i;

        strcpy(tmp, ldeq->arr[ind1]);
        strcpy(ldeq->arr[ind1], ldeq->arr[ind2]);
        strcpy(ldeq->arr[ind2], tmp);
    }
    free(tmp);
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
        fprintf(stderr, RED);
        perror("open error");
        fprintf(stderr, WHITE);
        exit(EXIT_FAILURE);
    }

    int fd1 = dup(STDOUT_FILENO);
    if(fd1 == -1) {
        fprintf(stderr, RED);
        perror("Error: file stream couldn't be duplicated");
        fprintf(stderr, WHITE);
        exit(EXIT_FAILURE);
    } 

    int fd2 = dup2(fd, STDOUT_FILENO);
    if(fd2 == -1){
        fprintf(stderr, RED);
        perror("Error: file stream couldn't be duplicated");
        fprintf(stderr, WHITE);
        exit(EXIT_FAILURE);
    }

    printf("%d\n", l_deq->size);
    
    show_log(l_deq);
    if(dup2(fd1, STDOUT_FILENO) == -1) {
        fprintf(stderr, RED);
        perror("Error: file stream couldn't be duplicated");
        fprintf(stderr, WHITE);
        exit(EXIT_FAILURE);
    }

    close(fd);
}

char *return_nth_recent(log_deque *ldeq, int n) {
    n--;
    if (n >= ldeq->size) {
        // exceeds history size : error
        fprintf(stderr, RED);
        perror("Error: invalid log access");
        fprintf(stderr, WHITE);
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
                fprintf(stderr, RED);
                perror("Error: index not specified");
                fprintf(stderr, WHITE);
                return -1;      // failure
            }

            id = atoi(tok);
            return id; // success
        } else {
            fprintf(stderr, RED);
            perror("Error: invalid log command");
            fprintf(stderr, WHITE);
            return -1;
        }
    }
}
