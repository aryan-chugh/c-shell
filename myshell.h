#ifndef MY_SHELL
#define MY_SHELL

#include "input.h"
#include "info.h"
#include "bg_handler.h"
#include "log.h"
#include "dir.h"
#include "reveal.h"
#include "proclore.h"
#include "seek.h"
#include "pipes.h"
#include "activities.h"
#include "ping.h"
#include "fg.h"
#include "iman.h"
#include "myshrc_parser.h"
#include "neonate.h"
#include "handlers.h"

#include "headers.h"
// Function prototypes

void run_command(char **list, int *codes, log_deque *ldeq, char *cmd, bool bg);
bool process_commands(char **list, int *codes, log_deque *ldeq, char *s);
void print_prompt();

#endif