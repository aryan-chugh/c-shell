#ifndef REVEAL_H
#define REVEAL_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include "dir.h"

#define RESET       "\033[0m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define WHITE       "\033[37m"
#define BOLD        "\033[1m"

typedef struct dirent dirent;

int compare_strings(const void *a, const void *b);
void print_file_info(char *path, struct stat *statbuf);
void show(char *path, int tags);
void execute_reveal(dir_tree *tree, char *cmd);

#endif