#include "info.h"

void getSystemName(sys_info* s) {
    struct utsname uts;
    if(uname(&uts) == 0) {
        int len = strlen(uts.nodename);
        s -> sys_name = (char *) malloc(sizeof(char) * (len + 5));
        if(s -> user_name == NULL) {
            fprintf(stderr, RED);
            perror("malloc");
            fprintf(stderr, WHITE);
            exit(EXIT_FAILURE);
        }
        strcpy(s -> sys_name, uts.nodename);
    }else { 
        s -> sys_name = (char *) malloc(sizeof(char) * (1));
        if(s -> sys_name == NULL) {
            fprintf(stderr, RED);
            perror("malloc");
            fprintf(stderr, WHITE);
            exit(EXIT_FAILURE);
        }
        s -> sys_name [0] = '\0';
        fprintf(stderr, RED);
        fprintf(stderr, "Error: couldn't read system name!\n");
        fprintf(stderr, WHITE);
    }
}

void getUsername(sys_info *s) {
    struct passwd *pw;

    int uid = getuid();
    pw = getpwuid(uid);

    if (pw) {
        int len = strlen(pw -> pw_name);
        s -> user_name = (char *) malloc(sizeof(char) * (len + 5));
        if(s -> user_name == NULL) {
            fprintf(stderr, RED);
            perror("malloc");
            fprintf(stderr, WHITE);
            exit(EXIT_FAILURE);
        }
        strcpy(s -> user_name, pw -> pw_name);
    } else {
        s -> user_name = (char *) malloc(sizeof(char) * (1));
        if(s -> user_name == NULL) {
            fprintf(stderr, RED);
            perror("malloc");
            fprintf(stderr, WHITE);
            exit(EXIT_FAILURE);
        }
        s -> user_name [0] = '\0';
        fprintf(stderr, RED);
        fprintf(stderr, "Error: couldn't read user name!\n");
        fprintf(stderr, WHITE);
    }
}
