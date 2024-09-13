#ifndef USERINFO_H
#define USERINFO_H

#include "headers.h"

typedef struct sys_info {
    char *sys_name;
    char *user_name;
}sys_info;

void getSystemName(sys_info* s);
void getUsername(sys_info* s);

#endif

// refine this file
