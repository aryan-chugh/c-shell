#include "seek.h"

int print_file_contents(const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        fprintf(stderr, RED);
        fprintf(stderr, "Error : couldn't open file.\n");
        fprintf(stderr, WHITE);

        return EXIT_FAILURE;
    }

    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
        fflush(stdout);
    }
    printf("\n");
    fflush(stdout);

    if (ferror(file)) {
        fprintf(stderr, RED);
        perror("ferror");
        fprintf(stderr, WHITE);

        return EXIT_FAILURE;
    }
    fclose(file);

    return EXIT_SUCCESS;
}

int search_directory(char *base_path, char *path_constructed, char *search_name, bool only_files, bool only_dir, int *counts, char **first_match, bool *file_found) {
    struct dirent *entry;
    DIR *dp = opendir(base_path);

    if (dp == NULL) {
        fprintf(stderr, RED);
        perror("opendir");
        fprintf(stderr, WHITE);
        return EXIT_FAILURE;
    }

    while ((entry = readdir(dp))) {
        // Skip the special entries "." and ".." (Assumption)
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construct the path to the current entry
        char path[4096];
        snprintf(path, sizeof(path), "%s/%s", base_path, entry->d_name);

        char rel_path[4096];
        snprintf(rel_path, sizeof(rel_path), "%s/%s", path_constructed, entry->d_name);

        struct stat statbuf;
        if (stat(path, &statbuf) != 0) {
            fprintf(stderr, RED);
            fprintf(stderr, "Error : file / folder details couldn't be checked!\n");
            fprintf(stderr, WHITE);
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            if (!only_files && strncmp(entry->d_name, search_name, strlen(search_name)) == 0){
                printf(BLUE "%s\n" WHITE, rel_path);
                if((*counts) == 0) {
                    strcpy((*first_match), path);
                    (*file_found) = false;
                }
                (*counts) ++;
            }
            search_directory(path, rel_path, search_name, only_files, only_dir, counts, first_match, file_found);
        } else {
            if (!only_dir && strncmp(entry->d_name, search_name, strlen(search_name)) == 0) {
                printf(GREEN "%s\n" WHITE, rel_path);
                
                if((*counts) == 0) {
                    strcpy((*first_match), path);
                    (*file_found) = true;
                }
                (*counts) ++;
            }
        }
    }

    closedir(dp);

    return EXIT_SUCCESS;
}

int take_actions(bool take_action, int *counts, char **first_match, bool isfile) {
    if((*counts) == 1 && take_action) {
        if(isfile) {
            if(access((*first_match), R_OK) == 0) {
                return print_file_contents((*first_match));
            }else {
                fprintf(stderr, RED);
                fprintf(stderr, "Error : Missing permissions for task.\n");
                fprintf(stderr, WHITE);
                return EXIT_FAILURE;
            }
        }else {
            if(access((*first_match), X_OK) == 0) {
                // here change the current working directory by calling hop to path
                char *args[1];
                args[0] = (*first_match);
                hop_to_paths(1, args);

                return EXIT_SUCCESS;
            }else {
                fprintf(stderr, RED);
                fprintf(stderr, "Error : Missing permissions for task.\n");
                fprintf(stderr, WHITE);
                return EXIT_FAILURE;
            }
        }
    }
    return EXIT_SUCCESS;
}

int execute_seek(char *cmd) {
    bool only_files = false;
    bool only_dir = false;
    bool is_action = false;
    
    char **paths = (char **) malloc(sizeof(char *) * 256);
    if(paths == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        return EXIT_FAILURE;
    }

    int paths_specified = 0;

    bool is_search_path = false ;
    char *search_path = (char *) malloc(sizeof(char) * 4096);

    if(search_path == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        return EXIT_FAILURE;
    }
    search_path[0] = '\0';

    char *substr = strtok(cmd, " ");
    while(substr != NULL) {
        if(substr[0] == '-') {
            if(strlen(substr) == 1) {
                paths[paths_specified] = (char *) malloc(sizeof(char) * 4096);
                if(paths[paths_specified] == NULL) {
                    fprintf(stderr, RED);
                    perror("malloc");
                    fprintf(stderr, WHITE);
                    return EXIT_FAILURE;
                }
                strcpy(paths[paths_specified++], substr);
            }

            for(int i = 1; i < strlen(substr); i ++) {
                if(substr[i] == 'f') {
                    if(only_dir) {
                        fprintf(stderr, RED);
                        fprintf(stderr, "Error : invalid command.\n");
                        fprintf(stderr, WHITE);
                        return EXIT_FAILURE;
                    }
                    only_files = true;
                }else if(substr[i] == 'd') {
                    if(only_files) {
                        fprintf(stderr, RED);
                        fprintf(stderr, "Error : invalid command.\n");
                        fprintf(stderr, WHITE);
                        return EXIT_FAILURE;
                    }
                    only_dir = true;
                }else if(substr[i] == 'e') {
                    is_action = true;
                }else {
                    fprintf(stderr, RED);
                    fprintf(stderr, "Error : unrecognised flag.\n");
                    fprintf(stderr, WHITE);
                    return EXIT_FAILURE;
                }
            }
        }else {
            if(is_search_path == false) {
                is_search_path = true;
                strcpy(search_path, substr);
            }else {
                paths[paths_specified] = (char *) malloc(sizeof(char) * 4096);
                if(paths[paths_specified] == NULL) {
                    fprintf(stderr, RED);
                    perror("malloc");
                    fprintf(stderr, WHITE);
                    return EXIT_FAILURE;
                }
                strcpy(paths[paths_specified ++], substr);
            }
        }
        substr = strtok(NULL, " ");
    }

    char *first_match = (char *) malloc(sizeof(char) * 4096);
    if(first_match == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        return EXIT_FAILURE;
    }
    int counts = 0;
    bool filefound = false;

    int status = 0;
    if(paths_specified == 0) {
        // get the current directory path from the dir_tree
        // convert to absolute 
        char *curr = get_complete_path();
        char initial_path[2] = ".\0";
        status = status | search_directory(curr, initial_path, search_path, only_files, only_dir, &counts, &first_match, &filefound);
    }else {
        char initial_path[2] = ".\0";
        for(int i = 0; i < paths_specified; i ++) {
            status = status | search_directory(convert_to_absolute(paths[i]), initial_path, search_path, only_files, only_dir, &counts, &first_match, &filefound);
        }
    }
    if(counts == 0) {
        fprintf(stderr, BLUE);
        fprintf(stderr, "No Match Found.\n");
        fprintf(stderr, WHITE);
    }else{
        status = status | take_actions(is_action, &counts, &first_match, filefound);
    }

    free(first_match);
    free(search_path);
    for(int i = 0; i < paths_specified; i++) {
        free(paths[i]);
    }
    free(paths);

    return status;
}