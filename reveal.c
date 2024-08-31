#include "reveal.h"

int calculate_total_blocks(char *path) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char full_path[4096];

    long long total_blocks = 0;

    // Open the directory
    if ((dir = opendir(path)) == NULL) {
        fprintf(stderr, RED);
        perror("opendir");
        fprintf(stderr, WHITE);
        return EXIT_FAILURE;
    }

    // Iterate over each entry in the directory
    while ((entry = readdir(dir)) != NULL) {
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        // Get file status
        if (stat(full_path, &file_stat) != 0) {
            fprintf(stderr, RED);
            perror("stat");
            fprintf(stderr, WHITE);
            continue;
        }

        // Add the number of blocks to the total
        total_blocks += file_stat.st_blocks;
    }

    // Close the directory
    closedir(dir);

    printf("Total: %lld\n", total_blocks / 2); // Divide by 2 to convert to 1K blocks
    return EXIT_SUCCESS;
}

int compare_strings(const void *a, const void *b) {
    const char *str1 = *(const char **)a;
    const char *str2 = *(const char **)b;
    return strcmp(str1, str2);
}

int print_file_info(char *path, struct stat *statbuf) {
    // File type and permissions
    char file_type[2] = {'?', '\0'};
    if (S_ISREG(statbuf->st_mode)) file_type[0] = '-';
    else if (S_ISDIR(statbuf->st_mode)) file_type[0] = 'd';
    else if (S_ISLNK(statbuf->st_mode)) file_type[0] = 'l';

    // Permissions
    char permissions[10];
    snprintf(permissions, sizeof(permissions), "%c%c%c%c%c%c%c%c%c",
        (statbuf->st_mode & S_IRUSR) ? 'r' : '-',
        (statbuf->st_mode & S_IWUSR) ? 'w' : '-',
        (statbuf->st_mode & S_IXUSR) ? 'x' : '-',
        (statbuf->st_mode & S_IRGRP) ? 'r' : '-',
        (statbuf->st_mode & S_IWGRP) ? 'w' : '-',
        (statbuf->st_mode & S_IXGRP) ? 'x' : '-',
        (statbuf->st_mode & S_IROTH) ? 'r' : '-',
        (statbuf->st_mode & S_IWOTH) ? 'w' : '-',
        (statbuf->st_mode & S_IXOTH) ? 'x' : '-');

    // Number of hard links
    int nlinks = (int)statbuf->st_nlink;

    // Owner name
    struct passwd *pw = getpwuid(statbuf->st_uid);
    const char *owner = pw ? pw->pw_name : "unknown";

    // Group name
    struct group *gr = getgrgid(statbuf->st_gid);
    const char *group = gr ? gr->gr_name : "unknown";

    // File size
    long int size = statbuf->st_size;

    // Last modification time
    char timebuf[256];
    struct tm *tm_info = localtime(&statbuf->st_mtime);
    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", tm_info);

    // Print file info with consistent column widths

    if(S_ISDIR(statbuf->st_mode)){
        printf(BLUE "%-1s %-9s %4d %-10s %-10s %8ld %s %s\n" WHITE,
            file_type, permissions, nlinks, owner, group, size, timebuf, path);
    }else if((statbuf->st_mode & S_IXUSR) || (statbuf->st_mode & S_IXGRP) || (statbuf->st_mode & S_IXOTH)) {
        printf(GREEN "%-1s %-9s %4d %-10s %-10s %8ld %s %s\n" WHITE,
            file_type, permissions, nlinks, owner, group, size, timebuf, path);
    }else if(S_ISREG(statbuf->st_mode)) {
        printf(WHITE "%-1s %-9s %4d %-10s %-10s %8ld %s %s\n" WHITE,
            file_type, permissions, nlinks, owner, group, size, timebuf, path);
    }
    fflush(stdout);

    return EXIT_SUCCESS;
}

// Function to show directory contents
int show(char *path, int tags) {
    DIR *dir;
    dir = opendir(path);
    if (dir == NULL) {
        fprintf(stderr, RED);
        perror("opendir");
        fprintf(stderr, WHITE);
        return EXIT_FAILURE;
    }

    if(tags & 1 == 1)
        calculate_total_blocks(path);

    dirent *entry; 
    char full_path[4096]; 
    struct stat statbuf;

    char **sub_dirs = (char **) malloc(sizeof(char *) * 256);
    if(sub_dirs == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        return EXIT_FAILURE;
    }

    int dirs = 0;
    while ((entry = readdir(dir)) != NULL) {
        sub_dirs[dirs] = (char *) malloc(sizeof(char) * 4096);
        if(sub_dirs[dirs] == NULL) {
            fprintf(stderr, RED);
            perror("malloc");
            fprintf(stderr, WHITE);
            return EXIT_FAILURE;
        }
        strcpy(sub_dirs[dirs++], entry->d_name);
    }

    qsort(sub_dirs, dirs, sizeof(char *), compare_strings);

    int ret_status = EXIT_SUCCESS;
    for(int i = 0; i < dirs; i++) {
        if(path[strlen(path) - 1] == '/')
            snprintf(full_path, sizeof(full_path), "%s%s", path, sub_dirs[i]);
        else
            snprintf(full_path, sizeof(full_path), "%s/%s", path, sub_dirs[i]);
        
        if (stat(full_path, &statbuf) == -1) {
            fprintf(stderr, RED);
            perror("Couldn't reach the file details");
            fprintf(stderr, WHITE);
            continue;
        }

        if(tags & 1) {
            if((tags >> 1) & 1 == 0 && sub_dirs[i][0] == '.') {
                continue;
            }
            ret_status = ret_status | print_file_info(full_path, &statbuf);
        } else {
            if((tags >> 1) & 1) {
                if(S_ISDIR((&statbuf)->st_mode)){
                    printf(BLUE "%s\n" WHITE, sub_dirs[i]);
                }else if(((&statbuf)->st_mode & S_IXUSR) || ((&statbuf)->st_mode & S_IXGRP) || ((&statbuf)->st_mode & S_IXOTH)) {
                    printf(GREEN "%s\n" WHITE, sub_dirs[i]);
                }else if(S_ISREG((&statbuf)->st_mode)) {
                    printf(WHITE "%s\n" WHITE, sub_dirs[i]);
                }
                fflush(stdout);
            } else {
                if(sub_dirs[i][0] != '.') {
                    if(S_ISDIR((&statbuf)->st_mode)){
                        printf(BLUE "%s\n" WHITE, sub_dirs[i]);
                    }else if(((&statbuf)->st_mode & S_IXUSR) || ((&statbuf)->st_mode & S_IXGRP) || ((&statbuf)->st_mode & S_IXOTH)) {
                        printf(GREEN "%s\n" WHITE, sub_dirs[i]);
                    }else if(S_ISREG((&statbuf)->st_mode)) {
                        printf(WHITE "%s\n" WHITE, sub_dirs[i]);
                    }
                    fflush(stdout);
                }
            }
        }
    }
    closedir(dir);

    for(int i = 0; i < dirs; i++) {
        free(sub_dirs[i]);
    }
    free(sub_dirs);

    return ret_status;
}

int show_file(char *path, int tags) {
    struct stat path_stat;

    // Retrieve information about the path
    if (stat(path, &path_stat) != 0) {
        fprintf(stderr, RED);
        perror("Couldn't reach the file details");
        fprintf(stderr, WHITE);
        return EXIT_FAILURE;
    }

    if(!S_ISDIR(path_stat.st_mode)) {
        if(tags & 1 == 1) 
            return print_file_info(path, &path_stat);
        else {
            char *tempor = (char *) malloc(sizeof(char) * 1024);
            if(tempor == NULL) {
                fprintf(stderr, RED);
                perror("malloc");
                fprintf(stderr, WHITE);

                return EXIT_FAILURE;
            }
            strcpy(tempor, path);
            printf("%s\n", basename(tempor));
        }
    }
    return EXIT_SUCCESS;
}

int check_path_and_show_details(char *path, int flags) {
    struct stat path_stat;

    if (stat(path, &path_stat) != 0) {
        fprintf(stderr, RED);
        perror("Couldn't reach the file details");
        fprintf(stderr, WHITE);
        return EXIT_FAILURE;
    }

    // Determine if it's a file or directory
    if (S_ISREG(path_stat.st_mode)) {
        return show_file(path, flags);
    } else if (S_ISDIR(path_stat.st_mode)) {
        return show(path, flags);
    } else {
        fprintf(stderr, RED);
        perror("Couldn't determine the file type!");
        fprintf(stderr, WHITE);
        return EXIT_FAILURE;
    }
}

// Function to parse commands and execute actions
int execute_reveal(dir_tree *tree, char *cmd) {
    int ext_stat = EXIT_SUCCESS;
    int flags = 0;

    char **paths = (char **) malloc(sizeof(char *) * 100);
    if(paths == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);

        return EXIT_FAILURE;
    }
    int paths_specified = 0;

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
            }else {
                for(int i = 1; i < strlen(substr); i++) {
                    if(substr[i] == 'a') {
                        flags |= 2;
                    } else if(substr[i] == 'l') {
                        flags |= 1;
                    } else {
                        fprintf(stderr, RED);
                        perror("Error : flags couldn't be recognised");
                        fprintf(stderr, WHITE);
                        return EXIT_FAILURE;
                    }
                }
            }
        } else {
            paths[paths_specified] = (char *) malloc(sizeof(char) * 4096);
            if(paths[paths_specified] == NULL) {
                fprintf(stderr, RED);
                perror("malloc");
                fprintf(stderr, WHITE);

                return EXIT_FAILURE;
            }
            strcpy(paths[paths_specified++], substr);
        }

        substr = strtok(NULL, " ");
    }

    if(paths_specified == 0) {
        // Get the current directory path from the dir_tree
        // Convert to absolute
        char *curr_dir = get_complete_path(tree);
        // Reveal
        ext_stat = ext_stat | show(curr_dir, flags);
        free(curr_dir);
    } else {
        for(int i = 0; i < paths_specified; i++) {
            // Convert the paths to absolute
            char *f_path_ = (char *) malloc(sizeof(char) * 256);
            if(f_path_ == NULL) {
                fprintf(stderr, RED);
                perror("malloc");
                fprintf(stderr, WHITE);
                return EXIT_FAILURE;
            }

            if(paths[i][0] == '/' || paths[i][0] == '-' || paths[i][0] == '~') {
                char * f_temp_ = convert_to_absolute(tree, paths[i]);
                strcpy(f_path_, f_temp_);
                free(f_temp_);
            }else {
                char *curr_dir = get_complete_path(tree);
                strcpy(f_path_, strcat(strcat(curr_dir, "/"), paths[i]));
            }

            // printf("%s\n", f_path_);
            ext_stat = ext_stat | check_path_and_show_details(f_path_, flags);
            free(f_path_);
        }
    }

    free(substr);
    for(int i = 0; i < paths_specified; i ++) {
        free(paths[i]);
    }
    free(paths);

    return ext_stat;
}
