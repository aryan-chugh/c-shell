#ifndef PROCLORE_H
#define PROCLORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "bg_handler.h"

#define RESET       "\033[0m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define WHITE       "\033[37m"
#define BOLD        "\033[1m"

char get_status(int pid);
int get_group(int pid);
unsigned long get_virtual_memory(int pid);
char *get_executable_path(int pid);
int check_background(bg_list *blist, int pid);

void execute_proclore(bg_list *blist, char *cmd, int shell_id);
void print_process(bg_list* blist, int pid);

#endif