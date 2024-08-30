#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "bg_handler.h"

// char get_status(int pid){ 
//     int flags = O_RDONLY;

//     char path[512];
//     snprintf(path, sizeof(path), "/proc/%d/status", pid);

//     int fd = open(path, flags);

//     if(fd == -1)  {
//         perror("open");
//         // error
//     }else {
//         int new_fd = dup(STDIN_FILENO);
//         dup2(fd, STDIN_FILENO);

//         char buffer[4096];
//         char state = '\0';
//         while (scanf("%[^\n]%*c", buffer) == 1) {
//             if(strncmp(buffer, "State", 5) == 0) {
//                 sscanf(buffer, "%*s%*c%c", &state);
//             } 
//         }
//         dup2(new_fd, STDIN_FILENO);
//         close(fd);
//         return state;
//     }
// }

char get_status(int pid) {
    int flags = O_RDONLY;

    char path[512];
    snprintf(path, sizeof(path), "/proc/%d/status", pid);

    int fd = open(path, flags);

    if (fd == -1) {
        perror("open");
        return '\0';  // Error handling: return a special value
    } else {
        char buffer[4096];
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        close(fd);  // Close the file descriptor after reading

        if (bytes_read == -1) {
            perror("read");
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

// int get_group(int pid){ 
//     int flags = O_RDONLY;

//     char path[512];
//     snprintf(path, sizeof(path), "/proc/%d/stat", pid);

//     int fd = open(path, flags);

//     if(fd == -1)  {
//         perror("open");
//         // error
//     }else {
//         int new_fd = dup(STDIN_FILENO);
//         dup2(fd, STDIN_FILENO);

//         int prc_grp = -1;
//         scanf("%*d %*s %*c %d", &prc_grp);

//         dup2(new_fd, STDIN_FILENO);
//         close(fd);
//         return prc_grp;
//     }
// }

int get_group(int pid) {
    int flags = O_RDONLY;

    char path[512];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    int fd = open(path, flags);

    if (fd == -1) {
        perror("open");
        return -1;  // Error handling: return a special value
    } else {
        char buffer[4096];
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        close(fd);  // Close the file descriptor as soon as we're done with it

        if (bytes_read == -1) {
            perror("read");
            return -1;  // Error handling: return a special value
        }

        buffer[bytes_read] = '\0';  // Null-terminate the buffer

        // Tokenize the buffer to get the desired field
        char *token;
        int field_count = 0;
        int prc_grp = -1;

        token = strtok(buffer, " ");
        while (token != NULL) {
            field_count++;
            if (field_count == 4) {
                prc_grp = atoi(token);  // Convert the token to an integer
                break;
            }
            token = strtok(NULL, " ");
        }

        return prc_grp;
    }
}

// unsigned long get_virtual_memory(int pid){ 
//     int flags = O_RDONLY;

//     char path[512];
//     snprintf(path, sizeof(path), "/proc/%d/statm", pid);

//     int fd = open(path, flags);

//     if(fd == -1)  {
//         perror("open");
//         // error
//     }else {
//         int new_fd = dup(STDIN_FILENO);
//         dup2(fd, STDIN_FILENO);

//         unsigned long vm;
//         scanf("%lu", &vm);

//         vm = vm * (unsigned long) getpagesize();

//         dup2(new_fd, STDIN_FILENO);
//         close(fd);
//         return vm;
//     }
// }


unsigned long get_virtual_memory(int pid) {
    int flags = O_RDONLY;

    char path[512];
    snprintf(path, sizeof(path), "/proc/%d/statm", pid);

    int fd = open(path, flags);

    if (fd == -1) {
        perror("open");
        return 0;  // Error handling: return a special value
    } else {
        char buffer[256];
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        close(fd);  // Close the file descriptor as soon as we're done with it

        if (bytes_read == -1) {
            perror("read");
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

char *get_executable_path(int pid) {
    char path[512];
    snprintf(path, sizeof(path), "/proc/%d/exe", pid);

    char link_path[4096]; // don't dynamically allocate here.
    ssize_t len = readlink(path, link_path, sizeof(link_path) - 1);
    if (len == -1) {
        // Error
        perror("readlink");
        return NULL;
    }
    link_path[len] = '\0'; // Null-terminate the link path

    char *ret_str = (char *) malloc(sizeof(char) * 4096);
    strcpy(ret_str, link_path);

    return ret_str;
}

int check_background(bg_list *blist, int pid) {
    if(get_name(blist, pid) == NULL) {
        pid_t pgid = getpgid(pid);
        if (pgid == -1) {
            perror("getpgid");
            return EXIT_FAILURE;
        }

        pid_t fg_pgid = tcgetpgrp(STDIN_FILENO);
        if (fg_pgid == -1) {
            if (errno == ENOTTY) {
                fprintf(stderr, "Not a terminal.\n");
            } else {
                perror("tcgetpgrp");
            }
            return -1;
        }

        if (pgid == fg_pgid) {
            return 0;   // is foreground
        } else {   
            return 1;   // is background
        }
    }else {
        printf("Reached here!\n");
        return 1;       // is background
    }
}

void print_process(bg_list* blist, int pid) {
    printf("pid : %d\n", pid);
    char isbg = check_background(blist, pid) == 1 ? '\0' : '+';
    printf("process status : %c%c\n", get_status(pid), isbg);
    printf("process group : %d\n", get_group(pid));
    printf("Virtual Memory : %lu\n", get_virtual_memory(pid));
    
    printf("executable path : %s\n", get_executable_path(pid));
    fflush(stdout);
}


void execute_proclore(bg_list *blist, char *cmd, int shell_id) {
    if (strlen(cmd) == 0) {
        print_process(blist, shell_id);
        // return 0; // success
    } else {
        char *tok = strtok(cmd, " ");
        if(tok == NULL) {
            print_process(blist, shell_id);
        }

        int pid = atoi(tok);
        print_process(blist, pid);
    }
}