#ifndef HEADERS_H
#define HEADERS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <errno.h>

#include <fcntl.h>
#include <dirent.h>

#include <time.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/statvfs.h>
#include <sys/wait.h>


#include <ctype.h>
#include <sys/utsname.h>
#include <pwd.h>
#include <grp.h>

#include <signal.h>

#include <termios.h>
#include <libgen.h>


#define RESET       "\033[0m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define WHITE       "\033[37m"
#define BOLD        "\033[1m"



typedef struct bg_process{
    int pid;
    char *name;
    char *command_name;
    int status; // 0 means not yet reaped, else 1 = normal, 2 = abnormal

    struct bg_process *next, *prev;
}bg_process;

typedef struct bg_list {
    int num;
    bg_process *head_lst;
} bg_list;

extern bg_list *blist;
extern int pid_foreground_curr;
extern char fg_name[4096]; // name of the command
extern char fg_comm[4096]; // terminal entered command



#endif