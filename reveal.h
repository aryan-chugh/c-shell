#ifndef REVEAL_H
#define REVEAL_H

#include "headers.h"
#include "dir.h"

typedef struct dirent dirent;

int compare_strings(const void *a, const void *b);
int print_file_info(char *path, struct stat *statbuf);
int show(char *path, int tags);
int execute_reveal(char *cmd);

#endif