#ifndef USERINFO_H
#define USERINFO_H


#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <pwd.h>
#include <unistd.h>
#include <string.h>

#define RESET       "\033[0m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define WHITE       "\033[37m"
#define BOLD        "\033[1m"

typedef struct sys_info {
    char *sys_name;
    char *user_name;
}sys_info;

void getSystemName(sys_info* s);
void getUsername(sys_info* s);

#endif

// refine this file
