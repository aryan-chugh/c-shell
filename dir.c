#include "dir.h"

#define CURRENT_DIR_FILE "current_dir.txt"
#define PREVIOUS_DIR_FILE "previous_dir.txt"
#define SHELL_HOME_FILE "shell_home.txt"

char *home_path;

void write_to_file(char *filename, char *content) {
    char *temp_file = (char *) malloc(sizeof(char) * 4096);
    if(temp_file == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);

        exit(EXIT_FAILURE);
    }
    strcpy(temp_file, home_path); 
    temp_file = strcat(strcat(temp_file, "/"), filename);

    FILE *file = fopen(temp_file, "w");
    /// there was the usage of filename2 here.

    // fprintf(stderr, "%s %s\n", filename, content);
    if (file != NULL) {
        fprintf(file, "%s", content);
        fclose(file);
    } else {
        fprintf(stderr, RED);
        perror("fopen");
        fprintf(stderr, WHITE);
    }

    free(temp_file);
}

char* read_from_file(int id) {
    char *buffer = (char *) malloc(sizeof(char) * 4096);
    if(buffer == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);

        exit(EXIT_FAILURE);
    }

    char *filename1 = (char *)malloc(sizeof(char) * 4096);
    if(filename1 == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);

        exit(EXIT_FAILURE);
    }

    if(id == 0) strcpy(filename1, CURRENT_DIR_FILE);
    else if (id == 1) strcpy(filename1, PREVIOUS_DIR_FILE);
    else if (id == 2) strcpy(filename1, SHELL_HOME_FILE);

    char *temp_file = (char *) malloc(sizeof(char) * 4096);
    if(temp_file == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);

        exit(EXIT_FAILURE);
    }

    strcpy(temp_file, home_path); 
    temp_file = strcat(strcat(temp_file, "/"), filename1);
    ///changed filename too

    FILE *file = fopen(temp_file, "r");
    if (file) {
        if (fgets(buffer, 4096, file) == NULL) {
            buffer[0] = '\0';  // Ensure the string is empty if reading fails
        }
        fclose(file);
    } else {
        fprintf(stderr, RED);
        perror("fopen");
        fprintf(stderr, WHITE);

        buffer[0] = '\0';  // Ensure the string is empty if file not found
        exit(EXIT_FAILURE);
    }
    buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character

    // fprintf(stderr, "%s %s\n", filename, buffer);
    return buffer;
}

void update_directories(char *new_dir, int prev_update) {
    if (prev_update) {
        char *current_dir = read_from_file(0);
        if(current_dir == NULL) {
            exit(EXIT_FAILURE);
        }
        write_to_file(PREVIOUS_DIR_FILE, current_dir);
    }
    write_to_file(CURRENT_DIR_FILE, new_dir);
}

void set_current_directory() {
    char *current_dir = read_from_file(0);
    if (current_dir == NULL || strlen(current_dir) == 0) {
        fprintf(stderr, RED);
        fprintf(stderr, "Error: Failed to read the current directory path.\n");
        fprintf(stderr, WHITE);
        return;
    }

    if (chdir(current_dir) == -1) {
        fprintf(stderr, RED);
        perror("chdir");
        fprintf(stderr, WHITE);
    } 
    free(current_dir);
}

void initialize_home() {
    char home_dir[4096];
    if (getcwd(home_dir, sizeof(home_dir)) == NULL) {
        fprintf(stderr, RED);
        perror("getcwd");
        fprintf(stderr, WHITE);
        exit(EXIT_FAILURE);
    }
    home_path = (char *) malloc(sizeof(char) * 4096);
    if(home_path == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);

        exit(EXIT_FAILURE);
    }
    strcpy(home_path, home_dir);

    write_to_file(SHELL_HOME_FILE, home_dir);
    write_to_file(CURRENT_DIR_FILE, home_dir);
    write_to_file(PREVIOUS_DIR_FILE, "");
}

#define PATH_MAX 4096
void hop_to_parent(int should_update) {
    if (chdir("..") == 0) {
        char new_dir[PATH_MAX];
        if (getcwd(new_dir, sizeof(new_dir)) != NULL) {
            update_directories(new_dir, should_update);
        } else {
            fprintf(stderr, RED);
            perror("getcwd");
            fprintf(stderr, WHITE);

            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, RED);
        perror("chdir");
        fprintf(stderr, WHITE);

        exit(EXIT_FAILURE);
    }
}

void hop_to_folder(char *folder, int should_update) {
    if (chdir(folder) == 0) {
        char new_dir[PATH_MAX];
        if (getcwd(new_dir, sizeof(new_dir)) != NULL) {
            update_directories(new_dir, should_update);
        } else {
            fprintf(stderr, RED);
            perror("getcwd");
            fprintf(stderr, WHITE);

            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, RED);
        perror("chdir");
        fprintf(stderr, WHITE);

        exit(EXIT_FAILURE);
    }
}

void hop_to_home(int should_update) {
    char *shell_home = read_from_file(2);
    if (chdir(shell_home) == 0) {
        update_directories(shell_home, should_update);
    } else {
        fprintf(stderr, RED);
        perror("chdir");
        fprintf(stderr, WHITE);

        exit(EXIT_FAILURE);
    }
}

void hop_to_cur() {
    // do nothing
}

void hop_to_root(int should_update) {
    if (chdir("/") == 0) {
        char root_dir[PATH_MAX];
        if (getcwd(root_dir, sizeof(root_dir)) != NULL) {
            update_directories(root_dir, should_update);
        } else {
            fprintf(stderr, RED);
            perror("getcwd");
            fprintf(stderr, WHITE);

            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, RED);
        perror("chdir");
        fprintf(stderr, WHITE);

        exit(EXIT_FAILURE);
    }
}

void hop_to_prev(int should_update) {
    char *prev_dir = read_from_file(1);
    if (prev_dir[0] != '\0') {
        // Attempt to change directory to the previous directory
        if (chdir(prev_dir) == 0) {
            char new_dir[4096];
            if (getcwd(new_dir, sizeof(new_dir)) != NULL) {
                update_directories(new_dir, should_update);  // Do not update previous directory again
            } else {
                fprintf(stderr, RED);
                perror("getcwd");
                fprintf(stderr, WHITE);

                exit(EXIT_FAILURE);
            }
        } else {
            fprintf(stderr, RED);
            perror("chdir");
            fprintf(stderr, WHITE);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, RED);
        fprintf(stderr, "hop: previous directory not set\n");
        fprintf(stderr, WHITE);
        exit(EXIT_FAILURE);
    }
}

char* get_complete_path() {
    return read_from_file(0);
}

char* get_curr_path() {
    char *current_dir = get_complete_path();
    char *shell_home = read_from_file(2);
    // printf("%s\n", shell_home);

    char *relative_path = (char *) malloc(sizeof(char) * 4096);
    if(relative_path == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);

        exit(EXIT_FAILURE);
    }
    
    // printf("%s %s %d\n", current_dir, shell_home, strncmp(current_dir, shell_home, strlen(shell_home)));
    if (strlen(current_dir) >= strlen(shell_home) && strncmp(current_dir, shell_home, strlen(shell_home)) == 0) {
        strcpy(relative_path + 1, current_dir + strlen(shell_home));
        relative_path[0] = '~';
        // snprintf(relative_path, sizeof(relative_path), "~%s", current_dir + strlen(shell_home));
    } else {
        strcpy(relative_path, current_dir);
    }
    return relative_path;
}

char* convert_to_absolute(char *path) {
    char resolved_path[PATH_MAX] = {0};
    char temp_path[PATH_MAX];
    strcpy(temp_path, path);

    char *token = strtok(temp_path, "/");
    if (path[0] == '/') {
        // Absolute path, start from root
        strcpy(resolved_path, "/");
    } else if (path[0] == '~') {
        // Home directory
        char *home_dir = read_from_file(2);
        strcpy(resolved_path, home_dir);
        token = strtok(NULL, "/");
    } else if (path[0] == '-') {
        // Previous directory
        char *prev_dir = read_from_file(1);
        strcpy(resolved_path, prev_dir);
        token = strtok(NULL, "/");
    } else {
        // Relative path, start from current directory
        char *curr_dir = read_from_file(0);
        strcpy(resolved_path, curr_dir);
    }

    while (token != NULL) {
        if (strcmp(token, "..") == 0) {
            // Move one directory up
            char *last_slash = strrchr(resolved_path, '/');
            if (last_slash != NULL && last_slash != resolved_path) {
                *last_slash = '\0';
            } else if (last_slash == resolved_path) {
                // If we're at the root, keep it as "/"
                resolved_path[1] = '\0';
            }
        } else if (strcmp(token, ".") != 0) {
            // Concatenate the directory or file to the path
            if (resolved_path[strlen(resolved_path) - 1] != '/') {
                strcat(resolved_path, "/");
            }
            strcat(resolved_path, token);
        }
        token = strtok(NULL, "/");
    }

    // Handle the case where the resolved path is empty (e.g., path was "..")
    if (strlen(resolved_path) == 0) {
        strcpy(resolved_path, "/");
    }

    return strdup(resolved_path);
}


int hop_to_paths(int argc, char *argv[]) {
    if (argc == 0) {
        hop_to_home(1);
        return 0;
    }

    for (int i = 0; i < argc; i++) {
        char *path_i = argv[i];
        char *token = strtok(path_i, "/");

        // If path starts with a slash, start from root
        if (path_i[0] == '/') {
            if (token == NULL) {
                hop_to_root(1);
                return 0;
            }else {
                hop_to_root(0);
            }
        }

        char *temp_curr = read_from_file(0);

        while (token != NULL) {
            if (strcmp(token, "~") == 0) {
                hop_to_home(0);
            } else if (strcmp(token, "..") == 0) {
                hop_to_parent(0);
            } else if (strcmp(token, ".") == 0) {
                // No action needed for current directory
            } else if (strcmp(token, "-") == 0) {
                hop_to_prev(0);
            } else {
                hop_to_folder(token, 0);
            }
            // Move to the next token
            token = strtok(NULL, "/");
        }
        
        // char *new_dir = getcwd(NULL, 0);
        // printf("%s\n", new_dir);
        // update_directories(new_dir, 0);
        write_to_file(PREVIOUS_DIR_FILE, temp_curr);

        // fprintf(stderr, "reached\n");
        // free(new_dir);
    }
    return 0;
}

int execute_hop(char *cmd) {
    char *tok = strtok(cmd, " ");
    int args = 0;
    char *argv[256];

    while(tok != NULL) {
        argv[args] = (char *) malloc(sizeof(char) * 4096);
        if(argv[args] == NULL) {
            fprintf(stderr, RED);
            perror("malloc");
            fprintf(stderr, WHITE);
            exit(EXIT_FAILURE);
        }
        strcpy(argv[args ++], tok);
        tok = strtok(NULL, " ");
    }

    int ret_ = hop_to_paths(args, argv);
    printf("%s\n", get_complete_path());
    fflush(stdout);
    return ret_;
}