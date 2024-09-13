#include "pipes.h"
// spaces should have been minimised (either the command or the pipes must be the first symbol) .... ensure
// is_prev_bg = false (when the previous is not bg or when the previous does not exist)
// all the commands are terminated by the '\0'

// ifd and ofd are passed by reference

int parse_with_pipes(char **list, int *codes, log_deque *ldeq, char *cmd, bool is_this_bg) {
    if(strlen(cmd) == 0) {
        return 0;
    }

    if(cmd[0] == '|') {
        fprintf(stderr, RED);
        fprintf(stderr, "Error: invalid use of pipes!\n");
        fprintf(stderr, WHITE);
        return -1;
    }

    char **pipe_cmd_list = (char **)malloc(sizeof(char *) * 1024);
    int cmd_count = 0;

    char *prev_ptr = cmd;
    char *curr_ptr;
    bool has_char_appeared = false;

    bool atleast_one_pipe = false;

    // Loop through the command string and split based on pipes
    while ((curr_ptr = strstr(prev_ptr, "|"))) {
        atleast_one_pipe = true;
        // Skip any leading spaces
        while (*prev_ptr == ' ') {
            prev_ptr++;
        }

        // Check if there is no command between pipes
        if (prev_ptr == curr_ptr) {
            fprintf(stderr, RED);
            fprintf(stderr, "Error: invalid use of pipes (empty command between pipes)!\n");
            fprintf(stderr, WHITE);
            // Clean up memory
            for (int i = 0; i < cmd_count; i++) {
                free(pipe_cmd_list[i]);
            }
            free(pipe_cmd_list);
            return -1;
        }

        // Allocate memory for the command
        pipe_cmd_list[cmd_count] = (char *)malloc(sizeof(char) * 4096);
        if (!pipe_cmd_list[cmd_count]) {
            fprintf(stderr, RED);
            perror("malloc");
            fprintf(stderr, WHITE);
            return -1;
        }

        // Copy the command between pipes
        strncpy(pipe_cmd_list[cmd_count], prev_ptr, curr_ptr - prev_ptr);
        pipe_cmd_list[cmd_count][curr_ptr - prev_ptr] = '\0';
        cmd_count++;

        prev_ptr = curr_ptr + 1;  // Move past the pipe
    }

    // Handle the last command after the final pipe
    while (*prev_ptr == ' ') {
        prev_ptr++;
    }

    // Check if there is a command after the last pipe
    if ((*prev_ptr == '\0') && atleast_one_pipe) {
        fprintf(stderr, RED);
        fprintf(stderr, "Error: invalid use of pipes (pipe at the end)!\n");
        fprintf(stderr, WHITE);
        // Clean up memory
        for (int i = 0; i < cmd_count; i++) {
            free(pipe_cmd_list[i]);
        }
        free(pipe_cmd_list);
        return -1;
    }

    // Allocate memory for the last command
    pipe_cmd_list[cmd_count] = (char *)malloc(sizeof(char) * 4096);
    if (!pipe_cmd_list[cmd_count]) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        return -1;
    }

    strcpy(pipe_cmd_list[cmd_count], prev_ptr);
    cmd_count++;

    // till here I have read the commands in the string excluding the pipes and arranged them in an array

    int original_inp = dup(STDIN_FILENO);
    if(original_inp == -1) {
        fprintf(stderr, RED);
        perror("dup");
        fprintf(stderr, WHITE);

        return -1;
    }
    int original_out = dup(STDOUT_FILENO);
    if(original_out == -1) {
        fprintf(stderr, RED);
        perror("dup");
        fprintf(stderr, WHITE);
        
        return -1;
    }

    int inp_fd = STDIN_FILENO;
    int out_fd = STDOUT_FILENO;
    
    // if(cmd_count == 1) {
    //     run_command(list, codes, ldeq, pipe_cmd_list[0], is_this_bg);
    //     return 0;
    // }

    int pipefd[2];
    for(int i = 0; i < cmd_count; i ++) {
        if(i < cmd_count - 1) {
            if(pipe(pipefd) == -1){
                fprintf(stderr, RED);
                perror("pipe");
                fprintf(stderr, WHITE);
                return -1;
            }
            out_fd = pipefd[1];
        }else {
            out_fd = original_out;
        }

        pipe_cmd_list[i] = minimise_spaces(pipe_cmd_list[i]);
        char *sv_ptr;
        char *cmd_to_send = strtok_r(strdup(pipe_cmd_list[i]), "<>", &sv_ptr);

        char *inp_change = strstr(pipe_cmd_list[i], "<");
        char *app_change = strstr(pipe_cmd_list[i], ">>"); 
        char *out_change = strstr(pipe_cmd_list[i], ">");

        if(inp_change != NULL) {
            inp_change += 1;
            bool is_valid = false;

            char *sv_ptr;
            char *token1 = strtok_r(inp_change, " ", &sv_ptr);
            if(token1 == NULL) {
                fprintf(stderr, RED);
                fprintf(stderr, "Error : Invalid! file redirection!\n");
                fprintf(stderr, WHITE);
                return -1;
            }
            char *inp_path = convert_to_absolute(token1);

            int flags = O_RDONLY;
            int fd = open(inp_path, flags);
            if(fd == -1) {
                fprintf(stderr, RED);
                perror("open");
                fprintf(stderr, WHITE);
                return -1;
            }
            if(i > 0) close(inp_fd);
            inp_fd = fd;
        }

        if(app_change != NULL) {
            app_change += 2;
            bool is_valid = false;

            char *sv_ptr;
            char *token1 = strtok_r(app_change, " ", &sv_ptr);
            if(token1 == NULL) {
                fprintf(stderr, RED);
                fprintf(stderr, "Error : Invalid! file redirection!\n");
                fprintf(stderr, WHITE);
                return -1;
            }
            char *app_path = convert_to_absolute(token1);

            int flags = O_CREAT | O_RDWR | O_APPEND;
            mode_t mode = 0644;
            int fd = open(app_path, flags, mode);
            if(fd == -1) {
                fprintf(stderr, RED);
                perror("open");
                fprintf(stderr, WHITE);
                return -1;
            }
            if(i < cmd_count - 1) close(out_fd);
            out_fd = fd;
        }else if(out_change != NULL) {
            out_change += 1;
            bool is_valid = false;

            char *sv_ptr;
            char *token1 = strtok_r(out_change, " ", &sv_ptr);
            if(token1 == NULL) {
                fprintf(stderr, RED);
                fprintf(stderr, "Error : Invalid! file redirection!\n");
                fprintf(stderr, WHITE);
                return -1;
            }
            char *out_path = convert_to_absolute(token1);

            int flags = O_CREAT | O_RDWR | O_TRUNC;
            mode_t mode = 0644;
            int fd = open(out_path, flags, mode);
            if(fd == -1) {
                fprintf(stderr, RED);
                perror("open");
                fprintf(stderr, WHITE);
                return -1;
            }
            if(i < cmd_count - 1) close(out_fd);
            out_fd = fd;
        }

        dup2(inp_fd, STDIN_FILENO);
        dup2(out_fd, STDOUT_FILENO);

        if(i < cmd_count - 1)
            run_command(list, codes, ldeq, cmd_to_send, false);
        else 
            run_command(list, codes, ldeq, cmd_to_send, is_this_bg);

        if(i > 0) {
            dup2(original_inp, STDIN_FILENO);
            close(inp_fd);
        }
        if(i < cmd_count - 1)  {
            dup2(original_out, STDOUT_FILENO);
            close(out_fd);
        }

        if(i < cmd_count - 1)
            inp_fd = pipefd[0];
    }

    dup2(original_inp, STDIN_FILENO);
    dup2(original_out, STDOUT_FILENO);
    return 0;
}