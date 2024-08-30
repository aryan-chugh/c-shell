#ifndef MY_SHELL
#define MY_SHELL

#include "dir.h"
#include "info.h"
#include "input.h"
#include "log.h"
#include "reveal.h"
#include "proclore.h"
#include "seek.h"
#include "bg_handler.h"

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

bool process_commands(char **list, int *codes, dir_tree *tree, log_deque *ldeq, bg_list* blist, char *s);

#endif