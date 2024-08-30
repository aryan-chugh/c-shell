#ifndef SEARCH_H
#define SEARCH_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
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


int print_file_contents(const char *file_path);
int take_actions(dir_tree *tree, bool take_action, int *counts, char **first_match, bool isfile);
int execute_seek(dir_tree *tree, char *cmd);
int search_directory(dir_tree *tree, char *base_path, char *path_constructed, char *search_name, bool only_files, bool only_dir, int *counts, char **first_match, bool *file_found);

#endif
