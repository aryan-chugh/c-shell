#ifndef __HANDLERS_H
#define __HANDLERS_H

#include "headers.h"
#include "bg_handler.h"
#include "ping.h"
#include "myshell.h"

void ctrl_c_handler(int signum);
void ctrl_d_handler();
void ctrl_z_handler(int signum);
void handle_signal(int signum);

#endif