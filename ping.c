#include "ping.h"

int send_ping(int pid, int sig, bool print) {
    // if(pid <= 0)  {
    //     fprintf(stderr, RED);
    //     fprintf(stderr, "Error: invalid pid!\n");
    //     fprintf(stderr, WHITE);
    //     return -1;
    // }

    if(sig < 0) {
        fprintf(stderr, RED);
        fprintf(stderr, "Error: invalid signal!\n");
        fprintf(stderr, WHITE);
        return -1;
    }

    sig = sig % 32;
    
    // fprintf("%d\n", pid);
    if(kill(pid, sig) == -1) {
        if(errno == EINVAL) {
            fprintf(stderr, RED);
            fprintf(stderr, "Error: Invalid signal!\n");
            fprintf(stderr, WHITE);
            return -1;
        }else if(errno == EPERM) {
            fprintf(stderr, RED);
            fprintf(stderr, "Error: Don't have permissions to execute task!\n");
            fprintf(stderr, WHITE);
            return -1;
        }else if(errno == ESRCH) {
            fprintf(stderr, RED);
            fprintf(stderr, "Error: No such process found!\n");
            fprintf(stderr, WHITE);
            return -1;
        }
    }else {
        if(print)
        printf("Sent signal %d to process with pid : %d\n", sig, pid);
        return 0;
    }
}

void execute_ping(char *cmd) {
    cmd = minimise_spaces(cmd);

    char *sv_ptr;
    char *tok = strtok_r(cmd, " ", &sv_ptr);
    int pd = atoi(tok);

    tok = strtok_r(NULL, " ", &sv_ptr);
    int sig = atoi(tok);

    int r_val = send_ping(pd, sig, true);
    if(r_val == -1) {
        exit(EXIT_FAILURE);
    }
}