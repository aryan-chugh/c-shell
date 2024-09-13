#ifndef __PIPING_H_
#define __PIPING_H_

#include "headers.h"
#include "log.h"
#include "myshell.h"

int parse_with_pipes(char **list, int *codes, log_deque *ldeq, char *cmd, bool is_this_bg);

#endif