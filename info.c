#include "info.h"

void getSystemName(sys_info* s) {
    struct utsname uts;
    if(uname(&uts) == 0) {
        int len = strlen(uts.nodename);
        s -> sys_name = (char *) malloc(sizeof(char) * (len + 5));
        strcpy(s -> sys_name, uts.nodename);
    }else { 
        // failure
    }
}

void getUsername(sys_info *s) {
    struct passwd *pw;

    int uid = getuid();
    pw = getpwuid(uid);

    if (pw) {
        int len = strlen(pw -> pw_name);
        s -> user_name = (char *) malloc(sizeof(char) * (len + 5));
        strcpy(s -> user_name, pw -> pw_name);
    } else {
        printf("No user information found.\n");
    }
}
