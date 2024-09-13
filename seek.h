#ifndef SEARCH_H
#define SEARCH_H

#include "headers.h"
#include "dir.h"

int print_file_contents(const char *file_path);
int take_actions(bool take_action, int *counts, char **first_match, bool isfile);
int execute_seek(char *cmd);
int search_directory(char *base_path, char *path_constructed, char *search_name, bool only_files, bool only_dir, int *counts, char **first_match, bool *file_found);

#endif