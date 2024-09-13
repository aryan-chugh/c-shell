#include "fg.h"

int is_pid_valid(pid_t pid) {
    if (kill(pid, 0) == 0) {
        return 1; // Process exists
    } else if (errno == ESRCH) {
        return 0; // No such process
    } else {
        perror("kill");
        return -1; // Error occurred
    }
}

int execute_fg(bg_list *list, char *cmd) {
    char *sv_ptr;
    char *tok = strtok_r(cmd, " ", &sv_ptr);

    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    int pd = atoi(tok);

    if (is_pid_valid(pd) && check_background(pd)) {
        if (kill(pd, SIGCONT) == -1) {
            fprintf(stderr, RED);
            perror("kill");
            fprintf(stderr, RED);
            return -1;
        }

        pid_foreground_curr = pd;
        strcpy(fg_name, get_process(blist, pd) -> name);
        strcpy(fg_comm, get_process(blist, pd) -> command_name); 
        remove_bg_process(list, pd);

        // int terminal_fd = open("/dev/tty", O_RDWR);
        // if (terminal_fd == -1) {
        //     perror("open");
        //     return -1;
        // }

        pid_t pgid = getpgid(pd);
        if (pgid == -1) {
            fprintf(stderr, RED);
            perror("getpgid");
            fprintf(stderr, WHITE);
            // close(terminal_fd);
            pid_foreground_curr = -1;
            fg_name[0] = '\0';
            fg_comm[0] = '\0';
            return -1;
        }

        // Set the PGID as the foreground process group for the terminal.
        if (tcsetpgrp(STDIN_FILENO, pgid) == -1) {
            fprintf(stderr, RED);
            perror("tcsetpgrp");
            fprintf(stderr, WHITE);
            // close(terminal_fd);
            pid_foreground_curr = -1;
            fg_name[0] = '\0';
            fg_comm[0] = '\0';
            return -1;
        }

        int status;
        do {
            pid_t wpid = waitpid(pd, &status, WUNTRACED);
            if (wpid == -1) {
                fprintf(stderr, RED);
                perror("waitpid");
                fprintf(stderr, WHITE);
                tcsetpgrp(STDIN_FILENO, getpgrp()); // Reset terminal on error
                // close(terminal_fd);
                pid_foreground_curr = -1;
                fg_name[0] = '\0';
                fg_comm[0] = '\0';
                return -1;
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status) && !WIFSTOPPED(status));

        // Reset the foreground process group to the shell.
        pid_foreground_curr = -1;
        fg_name[0] = '\0';
        fg_comm[0] = '\0';
        
        if (tcsetpgrp(STDIN_FILENO, getpgrp()) == -1) {
            fprintf(stderr, RED);
            perror("tcsetpgrp");
            fprintf(stderr, WHITE);
            return -1;
        }

        signal(SIGTTOU, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        // close(terminal_fd);

    } else {
        fprintf(stderr, RED);
        fprintf(stderr, "Error: No valid background process was found!\n");
        fprintf(stderr, WHITE);
        return -1;
    }
    return 0;
}


// int execute_fg(bg_list *list, char *cmd) {
//     char *sv_ptr;
//     char *tok = strtok_r(cmd, " ", &sv_ptr);

//     int pd = atoi(tok);

//     if(is_pid_valid(pd) && check_background(pd)) {
//         remove_bg_process(list, pd);

//         int r_val = send_ping(pd, SIGCONT, false);
//         if(r_val == -1) {
//             fprintf(stderr, "Couldn't resume the process!\n");
//             return -1;
//         }
        
//         int terminal_fd = open("/dev/tty", O_RDWR);
//         if (terminal_fd == -1) {
//             perror("open");
//         }
//         pid_t pgid = getpgid(pd);
//         if (pgid == -1)  {
//             perror("getpgid");
//         }

//         // Set the PGID as the foreground process group for the terminal.
//         // fprintf(stderr, "%c\n", get_status(pd));
//         if (tcsetpgrp(terminal_fd, pgid) == -1){
//             perror("tcsetpgrp");
//         }

//         int status;
//         do {
//             pid_t wpid = waitpid(pd, &status, WUNTRACED);
//             if (wpid == -1) {
//                 perror("waitpid");
//                 tcsetpgrp(terminal_fd, getpgrp()); // Reset terminal on error
//                 close(terminal_fd);
//                 return -1;
//             }
//         } while (!WIFEXITED(status) && !WIFSIGNALED(status) && !WIFSTOPPED(status));

//         // Reset the foreground process group to the shell.
//         if (tcsetpgrp(terminal_fd, getpgrp()) == -1) {
//             perror("tcsetpgrp");
//         }
//         close(terminal_fd);
//     }else {
//         fprintf(stderr, "Error: No valid background process was found!\n");
//         return -1;
//     }
// }


//-----------------------------------
// void execute_fg(bg_list *list, char *cmd) {
//     char *sv_ptr;
//     char *tok = strtok_r(cmd, " ", &sv_ptr);

//     int pd = atoi(tok);

//     if(is_pid_valid(pd) && check_background(pd)) {
//         remove_bg_process(list, pd);

//         // Set the foreground process group to the process
//         setpgid(0, 0);
//         tcsetpgrp(STDIN_FILENO, pd);

//         // Ignore terminal stop signals temporarily
//         signal(SIGTTOU, SIG_IGN);
//         signal(SIGTTIN, SIG_IGN);

//         // Send SIGCONT to the process to continue it
//         int r_val = send_ping(pd, SIGCONT, false);
//         if(r_val == -1) {
//             exit(EXIT_FAILURE);
//         }

//         // Restore default terminal stop signals
//         signal(SIGTTOU, SIG_DFL);
//         signal(SIGTTIN, SIG_DFL);

//         // Block the shell until the process finishes or stops
//         int status;
//         while (1) {
//             pid_t result = waitpid(pd, &status, WUNTRACED | WNOHANG);
//             if (result == -1) {
//                 if (errno == ECHILD) {
//                     // No more child processes
//                     break;
//                 }
//                 perror("waitpid");
//                 exit(EXIT_FAILURE);
//             }

//             // If process exited or was stopped, regain terminal control
//             if (WIFEXITED(status) || WIFSIGNALED(status)) {
//                 break;
//             } else if (WIFSTOPPED(status)) {
//                 tcsetpgrp(STDIN_FILENO, getpid());
//                 break;
//             }

//             // Sleep briefly to avoid busy-waiting
//             usleep(100000); // 100 ms
//         }

//         // Regain control of the terminal after the process stops or exits
//         tcsetpgrp(STDIN_FILENO, getpid());
//     } else {
//         fprintf(stderr, "Error: No valid background process was found!\n");
//         exit(EXIT_FAILURE);
//     }
// }

void execute_bg(char *cmd) {
    char *sv_ptr;
    char *tok = strtok_r(cmd, " ", &sv_ptr);

    int pd = atoi(tok);
    int r_val = send_ping(pd, SIGCONT, false);
    
    if(r_val == -1) {
        exit(EXIT_FAILURE);
    }
}