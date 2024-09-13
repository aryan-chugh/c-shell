#ifndef __FG_H_
#define __FG_H_
 
#include "headers.h"
#include "bg_handler.h"
#include "ping.h"
#include "proclore.h"

int is_pid_valid(pid_t pid);

int execute_fg(bg_list *list, char *cmd);
void execute_bg(char *cmd);

#endif