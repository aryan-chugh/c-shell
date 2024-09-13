#ifndef INPUT_H
#define INPUT_H

#include "headers.h"

#define MAX_COMMAND_LENGTH 4096
#define MAX_COMMANDS 100

char *minimise_spaces(const char *inp);
bool* mark_splits(char *str, char del1, char del2);
char **split(char *inp, char *delim);

#endif // INPUT_H
