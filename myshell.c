#include "myshell.h"
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

#define CMD_FILE "fcom.txt"
#define ALIAS_FILE ".myshrc"

bg_list *blist;

char *f_process[128];
int f_times[128];
int f_count = 0;
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

// cmd has no cmd part... 
void my_runner(char **list, int *codes, log_deque *ldeq, int indx, char *cmd) {
    cmd = minimise_spaces(cmd);
    switch(indx) {
        case 1:  // hop  
            execute_hop(cmd);
        break;
        case 2: // reveal
            execute_reveal(cmd);
        break;
        case 3: // log
            int rval = execute_log(cmd, ldeq);

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
            execute_proclore(blist, cmd, getpid());
            // execute_proclore(blist, cmd, getppid());
        break;
        case 5: // seek
            execute_seek(cmd);
        break;
        default:
            printf(RED "Unrecognised command!\n" WHITE);
    }
}

// cmd has cmd part
char **string_to_argv(const char *str) {
    char *str_copy = strdup(str);
    if (!str_copy) {
        fprintf(stderr, RED);
        perror("strdup");
        fprintf(stderr, WHITE);
        exit(EXIT_FAILURE);
    }

    size_t count = 0;
    char *token = strtok(str_copy, " ");
    while (token) {
        count++;
        token = strtok(NULL, " ");
    }

    char **argv = malloc((count + 1) * sizeof(char *));
    if (!argv) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        exit(EXIT_FAILURE);
    }

    strcpy(str_copy, str);
    size_t index = 0;

    token = strtok(str_copy, " ");
    while (token) {
        argv[index] = strdup(token);
        if (!argv[index]) {
            fprintf(stderr, RED);
            perror("strdup");
            fprintf(stderr, WHITE);
            exit(EXIT_FAILURE);
        }
        index++;
        token = strtok(NULL, " ");
    }

    // Null-terminate the argv array
    argv[index] = NULL;

    return argv;
}

void handle_signal(int signum) {
    int status;
    pid_t pid;

    // Reap the terminated child process
    pid = waitpid(-1, &status, WNOHANG);
    if (pid > 0) {
        bg_process *bprocess = get_process(blist, pid);
        if(bprocess == NULL) {
            fprintf(stderr, RED);
            fprintf(stderr, "Error: Background process couldn't be fetched!\n");
            fprintf(stderr, WHITE);
            return;
        }

        char *pname = get_name(blist, pid);
        if(pname == NULL) {
            fprintf(stderr, RED);
            fprintf(stderr, "Error: Background process name couldn't be fetched!\n");
            fprintf(stderr, WHITE);
        }

        if (WIFEXITED(status)) {
            bprocess -> status = 1;
        } else if (WIFSIGNALED(status)) {
            bprocess -> status = 2;
        } else {
            bprocess -> status = 3;
        }
    }
}

void printBuffer(){
    bg_process *bp = blist -> head_lst;

    int removepids[128];
    int count = 0;

    while(bp != NULL) {
        if(bp -> status != 0) {
            removepids[count ++] = bp -> pid;
            if(bp -> status == 1) {
                printf(BOLD "%s exited normally (%d)\n" RESET, bp->name, bp->pid);
            }else if(bp -> status == 2) {
                printf(BOLD "%s exited abnormally (%d)\n" RESET, bp->name, bp->pid);
            }else {
                printf(BOLD "%s exited with unknown status (%d)\n" RESET, bp->name, bp->pid);
            }
        }
        bp = bp -> next;
    }
    fflush(stdout);

    for(int i = 0; i < count; i ++) {
        remove_bg_process(blist, removepids[i]);
    }
}

void sys_runner(char *cmd) {
    char **argv = string_to_argv(cmd);
    if (execvp(argv[0], argv) == -1) {
        fprintf(stderr, RED);
        perror("execvp");
        fprintf(stderr, WHITE);

        free(argv);
        exit(EXIT_FAILURE);
    }
}

void run_command(char **list, int *codes, log_deque *ldeq, char *cmd, bool bg, int out_fd, int inp_fd) {
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

    struct tms start, end;
    clock_t start_time, end_time;
    long ticks_per_second = sysconf(_SC_CLK_TCK);

    // Record the start time
    start_time = times(&start);

    int pid = fork();

    if(pid == -1) {
        fprintf(stderr, RED);
        perror("fork");
        fprintf(stderr, WHITE);
    }else if(pid == 0) {
        usleep(100);

        // we permanently modify the input and output streams for the child
        if(inp_fd != -1) {
            if(dup2(inp_fd, STDIN_FILENO) == -1) {
                fprintf(stderr, RED);
                perror("Error: file stream exception");
                fprintf(stderr, WHITE);
                exit(EXIT_FAILURE);
            } 
        }
        if(out_fd != -1) {
            if(dup2(out_fd, STDOUT_FILENO) == -1) {
                fprintf(stderr, RED);
                perror("Error: file stream exception");
                fprintf(stderr, WHITE);
                exit(EXIT_FAILURE);
            }
        }

        int ind = -1;
        if((ind = find_in_predefined_list(list, codes, command)) == -1) {
            sys_runner(cmd2);
        }else {
            my_runner(list, codes, ldeq, ind, cmd_wo);
        }

        free(command);
        free(cmd);
        free(cmd2);

        exit(EXIT_SUCCESS);
    }else {
        if(bg) {
            add_bg_process(blist, pid, command);
            printf("[%d] %d\n", blist -> num, pid);
            fflush(stdout);
        }else {
            // loop again and again until we end up in the completion of the process
            // -- even if the process has been stopped but not completed run the loop.
            do {
                pid_t wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            
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

    int input_fd = STDIN_FILENO;
    int output_fd = STDOUT_FILENO;

    bool should_add_to_log = true;
    while(t[c] != NULL) {
        char *command = (char *) malloc(sizeof(char) * 4096);
        if(command == NULL) {
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
        }
        free(tok);

        should_add_to_log = should_add_to_log && (strcmp("log", command) != 0);
        run_command(list, codes, ldeq, t[c], m[c], output_fd, input_fd);
        
        // free(command);
        c ++;
    }

    return should_add_to_log;
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

    sys_info _sys_;
    getSystemName(&_sys_);
    getUsername(&_sys_);
    
    // home_path = (char *) malloc(sizeof(char) * 4096);
    
    // strcpy(home_path, get_complete_path());
    // printf("%s\n", home_path);

    signal(SIGCHLD, handle_signal);

    while(1) {
        char *pth = get_curr_path();
        char *processes = print_exceeds();
        printf("<%s@%s:%s%s> ", _sys_.user_name, _sys_.sys_name, strcmp(pth, "") == 0 ? "/" : pth, processes);
        fflush(stdout);

        char *user_input = (char *) malloc(sizeof(char) * 4096);
        if(user_input == NULL) {
            fprintf(stderr, RED);
            perror("malloc");
            fprintf(stderr, WHITE);
            return 1;
        }

        // handles the case where the input is just a newline character (scanf fails)
        if(scanf("%[^\n]%*c", user_input) == 0) {
            char c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            user_input[0] = '\0';
        }

        if(strcmp(user_input, "q") == 0) {
            free(user_input);
            break;
        }

        // printf("level - 1 : %s\n", user_input);
        bool hist = process_commands(cmds, cmd_codes, l_deque, user_input);
        if(hist) {
            add_command(user_input, l_deque);
        }

        printBuffer(); // exit statuses

        // free(pth);
        free(user_input);
        free(processes);
    }

    // after infinite loop
    // write_on_close(l_deque);    

    free(cmd_codes);
    for(int i = 0; cmds[i] != NULL; i ++) {
        free(cmds[i]);
    }
    free(cmds);
    return 0;
}