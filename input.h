#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

#define RESET       "\033[0m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define WHITE       "\033[37m"
#define BOLD        "\033[1m"

#define MAX_COMMAND_LENGTH 4096
#define MAX_COMMANDS 100

char *minimise_spaces(const char *inp);
bool* mark_splits(char *str, char del1, char del2);
char **split(char *inp, char *delim);

#endif // INPUT_H
