#include "myshell.h"
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

#define CMD_FILE ".fcom"
#define ALIAS_FILE ".myshrc"

bool process_commands(char **list, int *codes, dir_tree *tree, log_deque *ldeq, bg_list* blist, char *s);

// int *indx ==> contains the sequence number of the myshell-defined commands.
char **fetch_commands_from_file(int **indx) {
    char **cmds = (char **) malloc(sizeof(char *) * 256);   // allow 256 commands
    for(int i = 0; i < sizeof(cmds) / sizeof(char *); i ++) {
        cmds[i] = NULL;
    }

    FILE *file = fopen(CMD_FILE, "r");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    int cmd_count = 0;
            
    char *tmp = (char *) malloc(sizeof(char) * 256);
    while (fgets(buffer, sizeof(buffer), file) != NULL) { 
        char *ptr = buffer;  
        cmds[cmd_count] = (char *) malloc(sizeof(char) * 256);
    
        sscanf(ptr, "%s %d", tmp, &((*indx)[cmd_count]));
        strcpy(cmds[cmd_count ++], tmp);
    }
    return cmds;
}

int find_in_predefined_list(char **list, int *codes, char *cmd) {
    for(int i = 0; list[i] != NULL; i ++) {
        if(strcmp(list[i], cmd) == 0) {
            return codes[i];
        }
    }
    return -1;
}

// cmd has no cmd part... 
void my_runner(char **list, int *codes, bg_list *blist, log_deque *ldeq, dir_tree *tree, int indx, char *cmd) {
    switch(indx) {
        case 1:  // hop  
            execute_hop(tree, cmd);
        break;
        case 2: // reveal
            execute_reveal(tree, cmd);
        break;
        case 3: // log
            int rval = execute_log(cmd, ldeq);

            char *cmd_to_execute = return_nth_recent(ldeq, rval);
            if(cmd_to_execute == NULL) {
                return;
            }

            if(rval != -1 && rval != 0) {
                bool should_add = process_commands(list, codes, tree, ldeq, blist, cmd_to_execute);
                if(should_add) {
                    add_command(cmd_to_execute, ldeq);
                }
            }
        break;
        case 4: // proclore
            execute_proclore(blist, cmd, getpid());
            // execute_proclore(blist, cmd, shell_id);
        break;
        case 5: // seek
            execute_seek(tree, cmd);
        break;
        default:
            printf(RED "Unrecognised command!\n" WHITE);
    }
}

// cmd has cmd part
char **string_to_argv(const char *str) {
    char *str_copy = strdup(str);
    if (!str_copy) {
        perror("strdup");
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
        perror("malloc");
    }

    strcpy(str_copy, str);
    size_t index = 0;
    token = strtok(str_copy, " ");
    while (token) {
        argv[index] = strdup(token);
        if (!argv[index]) {
            perror("strdup");
        }
        index++;
        token = strtok(NULL, " ");
    }

    // Null-terminate the argv array
    argv[index] = NULL;

    return argv;
}

void sys_runner(char *cmd) {
    char **argv = string_to_argv(cmd);
 
    if (execvp(argv[0], argv) == -1) {
        printf(RED "Invalid command!\n" WHITE);
    }
}

void run_command(char **list, int *codes, log_deque *ldeq, bg_list *blist, dir_tree* tree, char *cmd, bool bg, int out_fd, int inp_fd) {
    int ppid = getpid();
    int status;

    char *command = (char *) malloc(sizeof(char) * 256);
    char *cmd_wo = (char *) malloc(sizeof(char) * 1024);
    
    char *cmd2 = (char *) malloc(sizeof(char) * 1024);
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
        cmd_wo[strlen(remaining_cmd)] = '\0'; // Ensure null-termination
    } else {
        cmd_wo[0] = '\0'; // If no remaining command, make it an empty string
    }

    int pid = fork();
    // signal... 
    if(pid == -1) {
        printf(RED "Error : Couldn't initiate the process!\n" WHITE);
    }else if(pid == 0) {
        usleep(100);

        // we permanently modify the input and output streams for the child
        if(inp_fd != -1) dup2(inp_fd, STDIN_FILENO);
        if(out_fd != -1) dup2(out_fd, STDOUT_FILENO);

        int ind = -1;
        // printf("%s\n", command);
        if((ind = find_in_predefined_list(list, codes, command)) == -1) {
            sys_runner(cmd2);
        }else {
            my_runner(list, codes, blist, ldeq, tree, ind, cmd_wo);
        }
    }else {
        if(bg) {
            add_bg_process(blist, pid, command);
            printf("[%d] %d\n", blist -> num, pid);
            return;
        }else {
            // loop again and again until we end up in the completion of the process
            // -- even if the process has been stopped but not completed run the loop.
            do {
                pid_t wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }
}

bool process_commands(char **list, int *codes, dir_tree *tree, log_deque *ldeq, bg_list* blist, char *s) {    
    char *p = minimise_spaces(s);
    if(strlen(p) == 0) {
        return false;
    }

    char *delim1 = (char *) malloc(sizeof(char) * 2);
    delim1[0] = ';';
    delim1[1] = '&';

    bool *m = mark_splits(p, delim1[0], delim1[1]);
    char **t = split(p, delim1);

    int c = 0;

    int input_fd = STDIN_FILENO;
    int output_fd = STDOUT_FILENO;

    bool should_add_to_log = true;
    while(t[c] != NULL) {
        // m[c] --> background
        char *command = (char *) malloc(sizeof(char) * 256);
        char *tok = (char *) malloc(sizeof(char) * 1024);
        strcpy(tok, t[c]);

        char *token = strtok(tok, " ");
        if (token != NULL) {
            strcpy(command, token);
            command[strlen(token)] = '\0'; // Ensure null-termination
        }
        free(tok);

        should_add_to_log = should_add_to_log && (strcmp("log", command) != 0);

        run_command(list, codes, ldeq, blist, tree, t[c], m[c], output_fd, input_fd);
        c ++;
    }

    return should_add_to_log;
}

int main() {
    int *cmd_codes = (int *) malloc(sizeof(int) * 256);
    char **cmds = fetch_commands_from_file(&cmd_codes);

    log_deque *l_deque = create_deq();
    fill_prev(l_deque);

    bg_list *prc_list = initialize_bg();    

    sys_info _sys_;
    getSystemName(&_sys_);
    getUsername(&_sys_);
    
    dir_tree *d_tree = (dir_tree *) malloc(sizeof(dir_tree));
    init_cwd(d_tree);

    while(1) {
        char *pth = get_curr_path(d_tree);
        printf("<%s@%s:%s> ", _sys_.user_name, _sys_.sys_name, strcmp(pth, "") == 0 ? "/" : pth);
        fflush(stdout);

        char *user_input = (char *) malloc(sizeof(char) * 4096);

        // handles the case where the input is just a newline character (scanf fails)
        if(scanf("%[^\n]%*c", user_input) == 0) {
            char c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            user_input[0] = '\0';
        }

        if(strcmp(user_input, "q") == 0) break;

        bool hist = process_commands(cmds, cmd_codes, d_tree, l_deque, prc_list, user_input);
        if(hist) {
            add_command(user_input, l_deque);
        }
        // add code to add the string user_input into the log given the log command is not in the string.
    }

    // after infinite loop
    write_on_close(l_deque);    
    return 0;
}