#include "bg_handler.h"

char get_status(int pid) {
    int flags = O_RDONLY;

    char path[512];
    snprintf(path, sizeof(path), "/proc/%d/status", pid);

    int fd = open(path, flags);

    if (fd == -1) {
        fprintf(stderr, RED);
        fprintf(stderr, "Error: Process status couldn't be fetched!\n");
        fprintf(stderr, WHITE);
        return '\0';  // Error handling: return a special value
    } else {
        char buffer[4096];
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        close(fd);  // Close the file descriptor after reading

        if (bytes_read == -1) {
            fprintf(stderr, RED);
            fprintf(stderr, "Error: Process status couldn't be fetched!\n");
            fprintf(stderr, WHITE);
            return '\0';  // Error handling: return a special value
        }

        buffer[bytes_read] = '\0';  // Null-terminate the buffer

        char *state_line = strstr(buffer, "State:");
        if (state_line != NULL) {
            state_line = strchr(state_line, ':');
            if (state_line != NULL) {
                state_line += 2;  // Skip the ": " after "State:"
                return *state_line;
            }
        }

        return '\0'; 
    }
}

int get_group(int pid) {
    int flags = O_RDONLY;

    char path[512];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    int fd = open(path, flags);

    if (fd == -1) {
        fprintf(stderr, RED);
        fprintf(stderr, "Error: Process group couldn't be fetched!\n");
        fprintf(stderr, WHITE);
        return -1;  // Error handling: return a special value
    } else {
        char buffer[4096];
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        close(fd);  // Close the file descriptor as soon as we're done with it

        if (bytes_read == -1) {
            fprintf(stderr, RED);
            fprintf(stderr, "Error: Process group couldn't be fetched!\n");
            fprintf(stderr, WHITE);
            return -1;  // Error handling: return a special value
        }

        buffer[bytes_read] = '\0';

        char *token;
        int field_count = 0;
        int prc_grp = -1;

        token = strtok(buffer, " ");
        while (token != NULL) {
            field_count++;
            // fprintf(stderr, "%d %s\n", field_count, token);
            if (field_count == 5) {
                prc_grp = atoi(token);  // Convert the token to an integer
                break;
            }
            token = strtok(NULL, " ");
        }

        return prc_grp;
    }
}

unsigned long get_virtual_memory(int pid) {
    int flags = O_RDONLY;

    char path[4096];
    snprintf(path, sizeof(path), "/proc/%d/statm", pid);

    int fd = open(path, flags);

    if (fd == -1) {
        fprintf(stderr, RED);
        fprintf(stderr, "Error: Process v. memory couldn't be fetched!\n");
        fprintf(stderr, WHITE);
        return 0;  // Error handling: return a special value
    } else {
        char buffer[4096];
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        close(fd);  // Close the file descriptor as soon as we're done with it

        if (bytes_read == -1) {
            fprintf(stderr, RED);
            fprintf(stderr, "Error: Process v. memory couldn't be fetched!\n");
            fprintf(stderr, WHITE);
            return 0;  // Error handling: return a special value
        }

        buffer[bytes_read] = '\0';  // Null-terminate the buffer

        // Manually parse the first field (virtual memory size)
        unsigned long vm = 0;
        char *endptr;
        vm = strtoul(buffer, &endptr, 10);

        // Convert pages to bytes
        vm = vm * (unsigned long)getpagesize();

        return vm;
    }
}

unsigned long get_virtual_memory2(int pid) {
    int flags = O_RDONLY;

    char path[512];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    int fd = open(path, flags);

    if (fd == -1) {
        fprintf(stderr, RED);
        fprintf(stderr, "Error: Process group couldn't be fetched!\n");
        fprintf(stderr, WHITE);
        return -1;  // Error handling: return a special value
    } else {
        char buffer[4096];
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        close(fd);  // Close the file descriptor as soon as we're done with it

        if (bytes_read == -1) {
            fprintf(stderr, RED);
            fprintf(stderr, "Error: Process group couldn't be fetched!\n");
            fprintf(stderr, WHITE);
            return -1;  // Error handling: return a special value
        }

        buffer[bytes_read] = '\0';

        char *token;
        int field_count = 0;

        unsigned long vm = 0;
        char *endptr;
        token = strtok(buffer, " ");
        while (token != NULL) {
            field_count++;
            // fprintf(stderr, "%d %s\n", field_count, token);
            if (field_count == 23) {
                // prc_grp = atoi(token);  // Convert the token to an integer
                vm = strtoul(token, &endptr, 10);
                break;
            }
            token = strtok(NULL, " ");
        }

        return vm;
    }
}

char *get_executable_path(int pid) {
    char path[512];
    snprintf(path, sizeof(path), "/proc/%d/exe", pid);

    char link_path[4096];
    ssize_t len = readlink(path, link_path, sizeof(link_path) - 1);
    if (len == -1) {
        fprintf(stderr, RED);
        fprintf(stderr, "Error: Process executable path couldn't be fetched!\n");
        fprintf(stderr, WHITE);
        return NULL;
    }
    link_path[len] = '\0';

    char *ret_str = (char *) malloc(sizeof(char) * 4096);
    if(ret_str == NULL) {
        fprintf(stderr, RED);
        fprintf(stderr, "Error: Couldn't allocate required memory!\n");
        fprintf(stderr, WHITE);
        return NULL;
    }
    strcpy(ret_str, link_path);

    return ret_str;
}

// int check_background(bg_list *blist, int pid) {
//     // if(get_name(blist, pid) == NULL) {
//         pid_t pgid = getpgid(pid);
//         if (pgid == -1) {
//             fprintf(stderr, RED);
//             fprintf(stderr, "Error: couldn't fetch the process group!\n");
//             fprintf(stderr, WHITE);
//             return EXIT_FAILURE;
//         }

//         pid_t fg_pgid = tcgetpgrp(STDIN_FILENO); // the process ground linked with the terminal
//         if (fg_pgid == -1) {
//             fprintf(stderr, RED);
//             if (errno == ENOTTY) {
//                 fprintf(stderr, "Not a terminal.\n");
//             } else {
//                 fprintf(stderr, "Error: gettind access to the terminal.\n");
//             }
//             fprintf(stderr, WHITE);
//             return -1;
//         }

//         if (pgid == fg_pgid) {
//             return 0;   // is foreground
//         } else {   
//             return 1;   // is background
//         }
//     // }
//     // else {
//     //     return 1;       // is background
//     // }
// }

int check_background(int pid) {
    char stat_file[4096];
    FILE *file;
    int process_grp = -1;
    int tpgid = -1;
    
    // Construct the path to the /proc/[pid]/stat file
    strcpy(stat_file, "/proc/");
    char pid_str[10];
    snprintf(pid_str, sizeof(pid_str), "%d", pid);
    strcat(stat_file, pid_str);
    strcat(stat_file, "/stat");

    // Open the file
    file = fopen(stat_file, "r");
    if (file != NULL) {
        char stat_line[5000];
        if (fgets(stat_line, sizeof(stat_line), file) != NULL) {
            // Parse the file content
            char list[52][30] = {0};
            int total = 0;
            char *delimiter = " :\t\n";
            char *token = strtok(stat_line, delimiter);

            while (token != NULL) {
                strncpy(list[total++], token, sizeof(list[0]) - 1);
                token = strtok(NULL, delimiter);
            }

            // Extract process group ID and terminal process group ID
            if (total > 7) {
                process_grp = atoi(list[4]);   // Process group ID
                tpgid = atoi(list[7]);         // Terminal process group ID
            }
        }
        fclose(file);
    } else {
        fprintf(stderr, "Process with %d is not valid\n", pid);
        return -1;
    }

    // fprintf(stderr, "%d %d\n", tpgid, process_grp);
    if(tpgid != process_grp) {
        return 1;
    } else {
        if(get_name(blist, pid) == NULL) {
            return 0;
        }else {
            return 1;
        }
    }   // is background?
}


int print_process(int pid) {
    int bg_stat = check_background(pid);
    char isbg =  bg_stat == 1 ? '\0' : bg_stat == -1 ? '\0' : '+';
    char status = get_status(pid);
    int pgroup = get_group(pid);
    unsigned long vm = get_virtual_memory(pid);

    char *executable = get_executable_path(pid);
    printf("pid : %d\n", pid);
    printf("process status : %c%c\n", status, isbg);
    printf("process group : %d\n", pgroup);
    printf("Virtual Memory : %lu\n", vm);
    
    printf("executable path : %s\n", executable);
    fflush(stdout);

    if(bg_stat == -1 || status == '\0' || pgroup == -1 || vm == 0 || executable == NULL) {
        if(executable != NULL) free(executable);
        return EXIT_FAILURE;
    }

    if(executable != NULL) free(executable);
    return EXIT_SUCCESS;
}


int execute_proclore(char *cmd, int shell_id) {
    int ret_code = EXIT_SUCCESS;
    if (strlen(cmd) == 0) {
        ret_code = ret_code | print_process(shell_id);
        return ret_code;
    } else {
        char *tok = strtok(cmd, " ");
        if(tok == NULL) {
            ret_code = ret_code | print_process(shell_id);
        }

        int pid = atoi(tok);
        ret_code = ret_code | print_process(pid);
    }
    return EXIT_SUCCESS;
}