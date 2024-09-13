#ifndef PROCLORE_H
#define PROCLORE_H

#include "headers.h"
#include "bg_handler.h"

char get_status(int pid);
int get_group(int pid);
unsigned long get_virtual_memory(int pid);
char *get_executable_path(int pid);
int check_background(int pid);

void execute_proclore(char *cmd, int shell_id);
void print_process(int pid);

#endif