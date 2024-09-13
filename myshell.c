#include "myshell.h"

#define CMD_FILE "fcom.txt"

bg_list *blist;

char *f_process[128];
int f_times[128];
int f_count = 0;

int pid_foreground_curr = -1;
char fg_name[4096]; // name of the command
char fg_comm[4096]; // terminal entered command

// char *home_path;

bool process_commands(char **list, int *codes, log_deque *ldeq, char *s);

// int *indx ==> contains the sequence number of the myshell-defined commands.
char **fetch_commands_from_file(int **indx) {
    char **cmds = (char **) malloc(sizeof(char *) * 256);   // allow 256 commands
    for(int i = 0; i < sizeof(cmds) / sizeof(char *); i ++) {
        cmds[i] = NULL;
    }

    FILE *file = fopen(CMD_FILE, "r");
    if (file == NULL) {
        fprintf(stderr, RED);
        fprintf(stderr, "Error: Couldn't fetch the available commands.\n");
        fprintf(stderr, WHITE);
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    int cmd_count = 0;
            
    char *tmp = (char *) malloc(sizeof(char) * 1024);
    if(tmp == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        exit(EXIT_FAILURE);
    }

    while (fgets(buffer, sizeof(buffer), file) != NULL) { 
        char *ptr = buffer;  
        cmds[cmd_count] = (char *) malloc(sizeof(char) * 1024);
        if(cmds[cmd_count] == NULL) {
            fprintf(stderr, RED);
            perror("malloc");
            fprintf(stderr, WHITE);
            exit(EXIT_FAILURE);
        }

        char *temp = strtok(ptr, " ");
        strcpy(tmp, temp);

        temp = strtok(NULL, " ");
        (*indx)[cmd_count] = atoi(temp);
        strcpy(cmds[cmd_count ++], tmp);
    }

    free(tmp);
    return cmds;
}

int find_in_predefined_list(char **list, int *codes, char *cmd) {
    cmd = minimise_spaces(cmd);
    for(int i = 0; list[i] != NULL; i ++) {
        if(strcmp(list[i], cmd) == 0) {
            return codes[i];
        }
    }
    return -1;
}

char **string_to_argv(char *str) {
    char *str_copy = strdup(str);
    if (!str_copy) {
        perror("strdup");
        exit(EXIT_FAILURE);
    }

    int count = 0;
    
    char **argv = (char **) malloc(sizeof(char *) * 4096);
    argv[0] = (char *) malloc(sizeof(char) * 4096);
    int indx = 0;

    bool is_in_quotes = false;
    for(int i = 0; i < strlen(str); i ++) {
        if(is_in_quotes) {
            if(str_copy[i] == '\'' || str_copy[i] == '"') {
                is_in_quotes = false;
                continue;
            }
            argv[count][indx ++] = str_copy[i];
            continue;
        }

        if(str_copy[i] == ' ') {
            argv[count++][indx ++] = '\0';
            argv[count] = (char *) malloc(sizeof(char) * 4096);
            indx = 0;
        }else {
            if(str_copy[i] == '\'' || str_copy[i] == '"') {
                is_in_quotes = true;
                continue;
            }
            argv[count][indx ++] = str_copy[i];
        }
    }
    if(indx > 0) {
        argv[count ++][indx ++] = '\0';
    }
    argv[count] = NULL;
    return argv;
}

char *print_exceeds() {
    if (f_count == 0) {
        char *empty = (char *)malloc(sizeof(char)); // Adjusted to accommodate "s" and null terminator
        if(empty == NULL) {
            fprintf(stderr, RED);
            perror("malloc");
            fprintf(stderr, WHITE);
            return NULL;
        }

        empty[0] = '\0';
        return empty;
    }

    // Start with a reasonably large buffer
    size_t buffer_size = 4096;
    char *to_print = (char *)malloc(buffer_size);
    if (to_print == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        return NULL;
    }
    to_print[0] = ' ';
    to_print[1] = '\0';

    int sum = 0;
    for (int i = 0; i < f_count; i++) {
        sum += f_times[i];

        // If the string is non-empty, add a space before appending the next part
        if (strlen(to_print) > 0 && i != 0) {
            strcat(to_print, " ");
        }

        // Format the string and append it to to_print
        char temp[256]; // temporary buffer for the formatted string
        sprintf(temp, "%s : %d%s", f_process[i], f_times[i], (i == f_count - 1) ? " :" : " ;");
        strcat(to_print, temp);

        // Reset the process and time
        f_times[i] = 0;
        free(f_process[i]);
    }

    // Add the sum at the end
    char sum_str[32];
    sprintf(sum_str, " %d", sum);
    strcat(to_print, sum_str);

    // Append "s" to the end of the string
    strcat(to_print, "s");

    f_count = 0;
    return to_print;
}

void print_prompt() {
    sys_info _sys_;
    getSystemName(&_sys_);
    getUsername(&_sys_);

    char *pth = get_curr_path();
    char *processes = print_exceeds();
    printf("<");
    printf(CYAN BOLD "%s@%s" RESET WHITE, _sys_.user_name, _sys_.sys_name);
    printf(":");
    printf(BLUE BOLD "%s%s" RESET WHITE, strcmp(pth, "") == 0 ? "/" : pth, processes);
    printf("> ");
    // printf("<%s@%s:%s%s> ", _sys_.user_name, _sys_.sys_name, strcmp(pth, "") == 0 ? "/" : pth, processes);       

    free(processes);
}

// cmd has no cmd part... 
void my_runner(char **list, int *codes, log_deque *ldeq, int indx, char *cmd) {
    cmd = minimise_spaces(cmd);
    
    char **argv = string_to_argv(cmd);
    char execute_cmd[4096];
    int len = 0;
    for(int i = 0; argv[i] != NULL; i ++) {
        strcpy(execute_cmd + len, argv[i]);
        len += strlen(argv[i]);
        execute_cmd[len ++] = ' ';
    }
    execute_cmd[len ++] = '\0';

    switch(indx) {
        case 1:  // hop  
            execute_hop(execute_cmd);
        break;
        case 2: // reveal
            execute_reveal(execute_cmd);
        break;
        case 3: // log
            int rval = execute_log(execute_cmd, ldeq);

            if(rval != -1 && rval != 0) {
                char *cmd_to_execute = (char *) malloc(sizeof(char) * 4096);
                if(cmd_to_execute == NULL) {
                    fprintf(stderr, RED);
                    perror("malloc");
                    fprintf(stderr, WHITE);

                    exit(EXIT_FAILURE);
                }
                strcpy(cmd_to_execute, return_nth_recent(ldeq, rval));
                if(cmd_to_execute == NULL) {
                    exit(EXIT_FAILURE);                // terminate the child process
                }

                bool should_add = process_commands(list, codes, ldeq, cmd_to_execute);
                if(should_add) {
                    add_command(cmd_to_execute, ldeq);
                }
                free(cmd_to_execute);
            }
        break;
        case 4: // proclore
            execute_proclore(execute_cmd, getpid());
            // execute_proclore(blist, cmd, getppid());
        break;
        case 5: // seek
            execute_seek(execute_cmd);
        break;
        case 6:
            print_activities(blist);
        break;
        case 7:
            execute_ping(execute_cmd);
        break;
        case 8:
            execute_fg(blist, execute_cmd);
        break;
        case 9:
            execute_bg(execute_cmd);
        break;
        case 10:
            execute_iman(execute_cmd);
        break;
        case 11 :
            execute_neonate(execute_cmd);
        break;
        default:
            printf(RED "Unrecognised command!\n" WHITE);
    }
}

// cmd has cmd part
// char **string_to_argv(const char *str) {
//     char *str_copy = strdup(str);
//     if (!str_copy) {
//         fprintf(stderr, RED);
//         perror("strdup");
//         fprintf(stderr, WHITE);
//         exit(EXIT_FAILURE);
//     }

//     size_t count = 0;
//     char *token = strtok(str_copy, " ");
//     while (token) {
//         count++;
//         token = strtok(NULL, " ");
//     }

//     char **argv = malloc((count + 1) * sizeof(char *));
//     if (!argv) {
//         fprintf(stderr, RED);
//         perror("malloc");
//         fprintf(stderr, WHITE);
//         exit(EXIT_FAILURE);
//     }

//     strcpy(str_copy, str);
//     size_t index = 0;

//     token = strtok(str_copy, " ");
//     while (token) {
//         argv[index] = strdup(token);
//         if (!argv[index]) {
//             fprintf(stderr, RED);
//             perror("strdup");
//             fprintf(stderr, WHITE);
//             exit(EXIT_FAILURE);
//         }
//         index++;
//         token = strtok(NULL, " ");
//     }

//     // Null-terminate the argv array
//     argv[index] = NULL;

//     return argv;
// }


// void disable_ctrl_echo() {
//     struct termios term;

//     // Get current terminal attributes
//     tcgetattr(STDIN_FILENO, &term);

//     // Disable ECHOCTL (if available) or equivalent behavior (control character echoing)
// // #ifdef ECHOCTL
//     term.c_lflag &= ~ECHOCTL;
//     term.c_lflag &= ~ISIG;
// // #else
// //     // Fallback: disable only control characters echo
// //     term.c_lflag &= ~(IEXTEN);
// // #endif

//     // Apply the new attributes
//     tcsetattr(STDIN_FILENO, TCSANOW, &term);
// }

// // Function to restore terminal settings
// void restore_terminal_settings_ctrl() {
//     struct termios term;
//     tcgetattr(STDIN_FILENO, &term);

//     // Re-enable control character echoing
// #ifdef ECHOCTL
//     term.c_lflag |= ECHOCTL;
// #else
//     // Fallback: re-enable control characters echo
//     term.c_lflag |= IEXTEN;
// #endif

//     tcsetattr(STDIN_FILENO, TCSANOW, &term);
// }

// can make a print_prompt to display a new prompt on Ctrl+C input

// void restore_fds_of_terminal() {
//     int dev_null = open("/dev/null", O_RDWR);
//     dup2(dev_null, STDIN_FILENO);
//     dup2(dev_null, STDOUT_FILENO);

//     // Reopen stdin and stdout from /dev/tty
//     int tty_in = open("/dev/tty", O_RDONLY);
//     int tty_out = open("/dev/tty", O_WRONLY);
//     dup2(tty_in, STDIN_FILENO);
//     dup2(tty_out, STDOUT_FILENO);

//     // Close the /dev/tty file descriptors
//     close(tty_in);
//     close(tty_out);
//     close(dev_null);
// }

// void ctrl_c_handler(int signum){
//     restore_fds_of_terminal();
//     printf("\n");
//     if(pid_foreground_curr == -1) {
//         print_prompt();
//         fflush(stdout);
//         return;
//     }

//     // fprintf(stderr, "reached here %d\n", pid_foreground_curr);
//     // fprintf(stderr, "%d\n", pid_foreground_curr);
//     int rval = send_ping(pid_foreground_curr, SIGINT, false);
//     pid_foreground_curr = -1;
//     fg_comm[0] = '\0';
//     fg_name[0] = '\0';
// }

// void ctrl_d_handler() {
//     restore_fds_of_terminal();
//     printf("\n");
    
//     if(pid_foreground_curr != -1) {
//         send_ping(pid_foreground_curr, SIGKILL, false);
//         pid_foreground_curr = -1;
//         fg_name[0] = '\0';
//         fg_comm[0] = '\0';
//     }

//     bg_process *bp = blist -> head_lst;

//     for(int i = 0; i < blist -> num; i ++) {
//         send_ping(bp ->pid, SIGKILL, false);
//         remove_bg_process(blist, bp->pid);
//     }

//     // fprintf(stderr, "reached here %d\n", pid_foreground_curr);

//     pid_foreground_curr = -1;
//     fg_comm[0] = '\0';
//     fg_name[0] = '\0';

//     exit(EXIT_SUCCESS);
// }

// void ctrl_z_handler(int signum) {
//     restore_fds_of_terminal();
//     printf("\n");
//     if(pid_foreground_curr == -1) {
//         print_prompt();
//         fflush(stdout);
//         return;
//     }
//     // send to bg and stop

//     // fprintf(stderr, "%d %d\n", getpgid(pid_foreground_curr), getpgid(getpid()));
//     // setpgid(pid_foreground_curr, 0);

//     send_ping(pid_foreground_curr, SIGTSTP, false);
//     send_ping(pid_foreground_curr, SIGSTOP, false);
//     // fprintf(stderr, "%d %d\n", getpgid(pid_foreground_curr), getpgid(getpid()));
//     // if (setpgid(pid_foreground_curr, 0) == -1) {
//     //     printf("Failed to set PGID: %s\n", strerror(errno));
//     //     exit(EXIT_FAILURE);
//     // }
//     // send_ping(pid_foreground_curr, SIGSTOP, false);

//     add_bg_process(blist, pid_foreground_curr, fg_name, fg_comm);
//     fprintf(stderr, "[%d] %d\n", blist -> num, pid_foreground_curr);

//     pid_foreground_curr = -1;
//     fg_comm[0] = '\0';
//     fg_name[0] = '\0';
// }

// void printBuffer(){
//     bg_process *bp = blist -> head_lst;

//     int removepids[128];
//     int count = 0;

//     while(bp != NULL) {
//         if(bp -> status != 0) {
//             removepids[count ++] = bp -> pid;
//             if(strlen(bp->name) == 0) {
//                 strcpy(bp -> name, "(empty)");
//             }
//             if(bp -> status == 1) {
//                 printf(BOLD "%s exited normally (%d)\n" RESET, bp->name, bp->pid);
//             }else if(bp -> status == 2) {
//                 printf(BOLD "%s exited abnormally (%d)\n" RESET, bp->name, bp->pid);
//             }else {
//                 printf(BOLD "%s exited with unknown status (%d)\n" RESET, bp->name, bp->pid);
//             }
//         }
//         bp = bp -> next;
//     }
//     fflush(stdout);

//     for(int i = 0; i < count; i ++) {
//         remove_bg_process(blist, removepids[i]);
//     }

//     // bg_process *bp2 = blist -> head_lst;
//     // while(bp2 != NULL) {
//     //     printf("%s\n", bp2 -> command_name);
//     //     bp2 = bp2 -> next;
//     // }
// }

void clean_blist() {
    int zombies[4096];
    int indx = 0;

    bg_process *bg_proc = blist -> head_lst;

    while(bg_proc != NULL) {
        int pid = bg_proc -> pid;
        if(get_status(pid) == 'Z') {
            if(strlen(bg_proc->name) == 0) {
                strcpy(bg_proc-> name, "(empty)");
            }
            printf(BOLD "%s was killed (%d)!\n" RESET, bg_proc->name, bg_proc->pid);

            zombies[indx ++] = pid;
        }
        bg_proc = bg_proc -> next;
    }

    for(int i = 0; i < indx; i ++) {
        remove_bg_process(blist, zombies[i]);
    }
}


// void handle_signal(int signum) {
//     int status;
//     pid_t pid;

//     // printf("aarha\n");
//     // fprintf(stderr, "%d\n", pid);
//     // bg_process *bp2 = blist -> head_lst;
//     // while(bp2 != NULL) {
//     //     printf("%s\n", bp2 -> command_name);
//     //     bp2 = bp2 -> next;
//     // }

//     while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
//         bg_process *bprocess = get_process(blist, pid);
//         if(bprocess == NULL) {
//             // means that a foreground process came here, this does not need to be handled. 
//             return;
//         }

//         // printf("arrha2\n");

//         if(strlen(bprocess->name) == 0) {
//             strcpy(bprocess -> name, "(empty)");
//         }
//         // fprintf(stderr, "%s\n", bprocess ->name);
//         if (WIFEXITED(status)) {
//             // bprocess -> status = 1;
//             printf(BOLD "%s exited normally (%d)\n" RESET, bprocess->name, bprocess->pid);
//             fflush(stdout);
//             remove_bg_process(blist, bprocess -> pid);
//         } 
//         else if (WIFSIGNALED(status)) {
//             // fprintf(stderr, "hello\n");
//             printf(BOLD "%s exited abnormally (%d)\n" RESET, bprocess->name, bprocess->pid);
//             fflush(stdout);
//             remove_bg_process(blist, bprocess -> pid);
//             // bprocess -> status = 2;
//         }
//         // Ignore if the child is stopped or continued
//         else if (WIFSTOPPED(status)) {
//             // Child was stopped by a signal, do not reap it
//             bprocess -> status = 0;
//         } 
//         else if (WIFCONTINUED(status)) {
//             bprocess -> status = 0;
//             // if(pid_foreground_curr != -1)
//             // send_ping(pid_foreground_curr, SIGCONT, false);
//         }
//     }

//     // printBuffer();
// }

int is_pipe_ready_for_reading(int pipe_fd) {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(pipe_fd, &read_fds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    int retval = select(pipe_fd + 1, &read_fds, NULL, NULL, &timeout);

    if (retval == -1) {
        perror("select");
        return -1;  // Error
    } else if (retval) {
        return 1;  // Data is available to read
    } else {
        return 0;  // No data available to read
    }
}

void sys_runner(char *cmd) {
    // char buffer[4096];
    // ssize_t bytes_read;
    // if() {
    //     while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer) - 1)) > 0) {
    //         buffer[bytes_read] = '\0'; // Null-terminate the buffer
    //         // fprintf(stderr, "Received: %s", buffer);
    //     }
    // }

    // fprintf(stderr, "%d\n", is_pipe_ready_for_reading(STDIN_FILENO));
    char **argv = string_to_argv(cmd);

    if (execvp(argv[0], argv) == -1) {
        fprintf(stderr, RED);
        perror("execvp");
        fprintf(stderr, WHITE);

        free(argv);
        exit(EXIT_FAILURE);
    }
}

void run_command(char **list, int *codes, log_deque *ldeq, char *cmd, bool bg) {
    cmd = minimise_spaces(cmd);
    int ppid = getpid();
    int status;

    char *command = (char *) malloc(sizeof(char) * 4096);
    if(command == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        return;
    }

    char *cmd_wo = (char *) malloc(sizeof(char) * 4096);
    if(cmd_wo == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        return;
    }
    
    char *cmd2 = (char *) malloc(sizeof(char) * 4096);
    if(cmd2 == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        return;
    }

    strcpy(cmd2, cmd);

    char *token = strtok(cmd, " ");
    if (token != NULL) {
        strcpy(command, token);
        command[strlen(token)] = '\0'; // Ensure null-termination
    } else {
        command[0] = '\0'; // If no command, make it an empty string
    }

    // Get the rest of the command (cmd_wo)
    char *remaining_cmd = strtok(NULL, "\n");
    if (remaining_cmd != NULL) {
        strcpy(cmd_wo, remaining_cmd);
        cmd_wo[strlen(remaining_cmd)] = '\0';
    } else {
        cmd_wo[0] = '\0'; // If no remaining command, make it an empty string
    }

    // we have the cmd_wo (containing the remaining string except the command name)

    struct tms start, end;
    clock_t start_time, end_time;
    long ticks_per_second = sysconf(_SC_CLK_TCK);

    // Record the start time
    start_time = times(&start);

    // int original_stdin = dup(STDIN_FILENO);
    // int original_stdout = dup(STDOUT_FILENO);

    // if(inp_fd != -1) {
    //     if(dup2(inp_fd, STDIN_FILENO) == -1) {
    //         fprintf(stderr, RED);
    //         perror("Error: file stream exception");
    //         fprintf(stderr, WHITE);
    //         exit(EXIT_FAILURE);
    //     } 
    // }
    // if(out_fd != -1) {
    //     if(dup2(out_fd, STDOUT_FILENO) == -1) {
    //         fprintf(stderr, RED);
    //         perror("Error: file stream exception");
    //         fprintf(stderr, WHITE);
    //         exit(EXIT_FAILURE);
    //     }
    // }
    int pid = fork();

    if(pid == -1) {
        fprintf(stderr, RED);
        perror("fork");
        fprintf(stderr, WHITE);
    }else if(pid == 0) {
        usleep(100);
        // signal(SIGTTOU, SIG_IGN);  // Ignore output signal for background processes
        // signal(SIGTTIN, SIG_IGN);
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        // signal(SIGCHLD, SIG_IGN);

        // disable_ctrl_echo();

        int ind = -1;
        if(bg) {
            if (setpgid(0, 0) == -1) {
                printf("Failed to set PGID: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }

        if((ind = find_in_predefined_list(list, codes, command)) == -1) {
            // fprintf(stderr, "%s\n", cmd2);
            sys_runner(cmd2);
        }else {
            my_runner(list, codes, ldeq, ind, cmd_wo);
            // signal(SIGTSTP, SIG_DFL);
        }

        free(command);
        free(cmd);
        free(cmd2);

        exit(EXIT_SUCCESS);
    }else {
        // if(dup2(original_stdin, STDIN_FILENO) == -1) {
        //     fprintf(stderr, RED);
        //     perror("Error: file stream exception");
        //     fprintf(stderr, WHITE);
        //     exit(EXIT_FAILURE);
        // } 
        // if(dup2(original_stdout, STDOUT_FILENO) == -1) {
        //     fprintf(stderr, RED);
        //     perror("Error: file stream exception");
        //     fprintf(stderr, WHITE);
        //     exit(EXIT_FAILURE);
        // }
        if(bg) {
            setpgid(pid, pid);
            add_bg_process(blist, pid, command, cmd2);
            fprintf(stderr, "[%d] %d\n", blist -> num, pid);
        }else {
            // loop again and again until we end up in the completion of the process
            // -- even if the process has been stopped but not completed run the loop.
            
            // setpgid(pid, pid);
            // tcsetpgrp(STDIN_FILENO, pid);

            // fprintf(stderr, "%d\n", pid);
            pid_foreground_curr = pid;
            strcpy(fg_comm, cmd2);
            strcpy(fg_name, command);

            do {
                pid_t wpid = waitpid(pid, &status, WUNTRACED);
                if (wpid == -1) {
                    fprintf(stderr, RED);
                    perror("waitpid");
                    fprintf(stderr, WHITE);
                    return;
                }
            } while (!WIFEXITED(status) && !WIFSIGNALED(status) && !WIFSTOPPED(status));
            // we can remove the WIFSTOPPED if we want to wait for the stopped processes.
            
            pid_foreground_curr = -1;
            fg_comm[0] = '\0';
            fg_name[0] = '\0';

            // tcsetpgrp(STDIN_FILENO, getpid());      // re-establish my terminal as the foreground process.
            set_current_directory();

            end_time = times(&end);
            double elapsed_time = (double)(end_time - start_time) / ticks_per_second;

            int measure = (int) elapsed_time;
            if(measure > 2) {
                f_process[f_count] = (char *) malloc(sizeof(char) * 1024);
                strcpy(f_process[f_count], command);
                f_times[f_count] = measure;
                f_count ++;
            }
        }

        free(command);
        free(cmd);
        free(cmd2);
    }
}

bool process_commands(char **list, int *codes, log_deque *ldeq, char *s) {    
    char *p = minimise_spaces(s);

    if(p == NULL) {
        return true;
    }
    if(strlen(p) == 0) {
        return false;
    }

    p = modify_for_aliases(p);
    // fprintf(stderr, "%s\n", p);
    p = modify_for_functions(p);
    // fprintf(stderr, "%s\n", p);
    char *delim1 = (char *) malloc(sizeof(char) * 3);
    if(delim1 == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        return true;
    }

    delim1[0] = ';';
    delim1[1] = '&';
    delim1[2] = '\0';
    bool *m = mark_splits(p, delim1[0], delim1[1]);
    if(m == NULL) {
        return true;
    }

    char **t = split(p, delim1);
    if(t == NULL) {
        return true;
    }

    int c = 0;

    bool should_add_to_log = true;
    while(t[c] != NULL) {
        char *command = (char *) malloc(sizeof(char) * 4096);
        if(command == NULL) {
            fprintf(stderr, RED);
            perror("malloc");
            fprintf(stderr, WHITE);
            return true;
        }
        char *rem_cmd = (char *) malloc(sizeof(char) * 4096);
        if(rem_cmd == NULL) {
            fprintf(stderr, RED);
            perror("malloc");
            fprintf(stderr, WHITE);
            return true;
        }

        char *tok = (char *) malloc(sizeof(char) * 4096);
        if(tok == NULL) {
            fprintf(stderr, RED);
            perror("malloc");
            fprintf(stderr, WHITE);
            return true;
        }
        strcpy(tok, t[c]);

        char *token = strtok(tok, " ");
        if (token != NULL) {
            strcpy(command, token);
            command[strlen(token)] = '\0'; // Ensure null-termination
        }else {
            command[0] = '\0';
        }
 
        token = strtok(NULL, "\n");
        if (token != NULL) {
            strcpy(rem_cmd, token);
            rem_cmd[strlen(token)] = '\0'; // Ensure null-termination
        }else {
            rem_cmd[0] = '\0';
        }

        free(tok);

        should_add_to_log = should_add_to_log && (strcmp("log", command) != 0);

        // somewhat redundant code, but makes fg run in the parent process only.
        if(strcmp("fg", command) == 0) {
            struct tms start1, end1;
            clock_t start_time1, end_time1;

            long ticks_per_second = sysconf(_SC_CLK_TCK);
            start_time1 = times(&start1);

            execute_fg(blist, rem_cmd);

            set_current_directory();

            end_time1 = times(&end1);
            double elapsed_time = (double)(end_time1 - start_time1) / ticks_per_second;

            int measure = (int) elapsed_time;
            
            if(measure > 2) {
                f_process[f_count] = (char *) malloc(sizeof(char) * 1024);
                strcpy(f_process[f_count], command);
                f_times[f_count] = measure;
                f_count ++;
            }
            c ++;
            continue;
        }

        // add the layer of parsing with pipes --> then it sends out the run_command
        t[c] = minimise_spaces(t[c]);
        int return_code = parse_with_pipes(list, codes, ldeq, t[c], m[c]);
        // run_command(list, codes, ldeq, t[c], m[c], output_fd, input_fd);
        // free(command);
        c ++;
    }

    return should_add_to_log;
}


int main() {
    int *cmd_codes = (int *) malloc(sizeof(int) * 256);
    if(cmd_codes == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        return 1;
    }

    char **cmds = fetch_commands_from_file(&cmd_codes);

    initialize_home();

    log_deque *l_deque = create_deq();
    fill_prev(l_deque);

    blist = initialize_bg();    

    // sys_info _sys_;
    // getSystemName(&_sys_);
    // getUsername(&_sys_);
    
    // home_path = (char *) malloc(sizeof(char) * 4096);
    
    // strcpy(home_path, get_complete_path());
    // printf("%s\n", home_path);

    signal(SIGCHLD, handle_signal);

    // signal(SIGTTOU, SIG_IGN);  // Ignore output signal for background processes
    // signal(SIGTTIN, SIG_IGN);

    // disable_ctrl_echo();
    signal(SIGINT, ctrl_c_handler);
    signal(SIGTSTP, ctrl_z_handler);

    // signal(SIGALRM, clean_blist);
    // alarm(10);

    while(1) {
        // char *pth = get_curr_path();
        // char *processes = print_exceeds();
        // printf("<%s@%s:%s%s> ", _sys_.user_name, _sys_.sys_name, strcmp(pth, "") == 0 ? "/" : pth, processes);
        print_prompt();
        // fflush(stdin);
        fflush(stdout);

        // char *user_input = (char *) malloc(sizeof(char) * 4096);
        // if(user_input == NULL) {
        //     fprintf(stderr, RED);
        //     perror("malloc");
        //     fprintf(stderr, WHITE);
        //     return 1;
        // }

        char user_input[4096];
        // handles the case where the input is just a newline character (scanf fails)
        // if(scanf("%[^\n]%*c", user_input) == 0) {
        //     char c;
        //     while ((c = getchar()) != '\n' && c != EOF) {}
        //     user_input[0] = '\0';
        // }

        if (fgets(user_input, sizeof(user_input), stdin) == NULL) {
            // Handle EOF or error
            if (feof(stdin)) {
                // End-of-file (EOF) was encountered
                user_input[0] = '\0';  // Set the string to empty
                break;
                // ctrl_d_handler();
                // fflush(stdin);
            } else if (ferror(stdin)) {
                // An error occurred during input
                fprintf(stderr, RED);
                perror("Error reading input");
                fprintf(stderr, WHITE);
                exit(EXIT_FAILURE);
            }
        } else {
            // Remove the trailing newline character if present
            size_t len = strlen(user_input);
            if (len > 0 && user_input[len - 1] == '\n') {
                user_input[len - 1] = '\0';
            }
        }

        // Output the processed input for demonstration
        // printf("Processed input: '%s'\n", user_input);

        // if (scanf("%[^\n]%*c", user_input) == EOF) {
        //     // Detect Ctrl+D
        //     if (feof(stdin)) {
        //         printf("\nEOF detected (Ctrl+D pressed). Exiting...\n");
        //         ctrl_d_handler();
        //     }
        // } else if (scanf("%[^\n]%*c", user_input) == 0) {
        //     // Handle case where input is just a newline character
        //     char c;
        //     while ((c = getchar()) != '\n' && c != EOF) {}

        //     user_input[0] = '\0';
        //     printf("Input was empty\n");
        // }

        fflush(stdout);
        // fprintf(stderr, "%d\n", user_input[0]);
        // sleep(2);

        // printBuffer();

        bool hist = process_commands(cmds, cmd_codes, l_deque, user_input);
        if(hist) {
            add_command(user_input, l_deque);
        }

        // free(user_input);
        // free(processes);
    }    

    free(cmd_codes);
    for(int i = 0; cmds[i] != NULL; i ++) {
        free(cmds[i]);
    }
    free(cmds);
    // alarm(0);
    ctrl_d_handler();
    return 0;
}