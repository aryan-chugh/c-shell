#ifndef PING_H
#define PING_H

#include "headers.h"
#include "input.h"

int send_ping(int pid, int sig, bool print);
void execute_ping(char *cmd) ;
#endif