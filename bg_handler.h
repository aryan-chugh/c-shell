#ifndef BG_HANDLER_H
#define BG_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESET       "\033[0m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define WHITE       "\033[37m"
#define BOLD        "\033[1m"


typedef struct bg_process{
    int pid;
    char *name;

    struct bg_process *next, *prev;
}bg_process;

typedef struct bg_list {
    int num;
    bg_process *head_lst;
} bg_list;


bg_list *initialize_bg();
void add_bg_process(bg_list *lst, int pid, char *proc_name);
void remove_bg_process(bg_list *lst, int pid);

char *get_name(bg_list *lst, int pid);
int get_num(bg_list *lst);

#endif