#ifndef BG_HANDLER_H
#define BG_HANDLER_H

#include "headers.h"

// typedef struct bg_process{
//     int pid;
//     char *name;
//     char *command_name;
//     int status; // 0 means not yet reaped, else 1 = normal, 2 = abnormal

//     struct bg_process *next, *prev;
// }bg_process;

// typedef struct bg_list {
//     int num;
//     bg_process *head_lst;
// } bg_list;


bg_list *initialize_bg();
void add_bg_process(bg_list *lst, int pid, char *proc_name, char *command_name);
void remove_bg_process(bg_list *lst, int pid);

char *get_name(bg_list *lst, int pid);
bg_process *get_process(bg_list *lst, int pid);
int get_num(bg_list *lst);
void free_bg_list(bg_list *lst);
#endif