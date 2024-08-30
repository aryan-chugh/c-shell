#include "reveal.h"

int compare_strings(const void *a, const void *b) {
    const char *str1 = *(const char **)a;
    const char *str2 = *(const char **)b;
    return strcmp(str1, str2);
}

void print_file_info(char *path, struct stat *statbuf) {
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
}

// Function to show directory contents
void show(char *path, int tags) {
    DIR *dir;
    dir = opendir(path);
    if (dir == NULL) {
        // Error opening directory
        printf(RED "Error : Invalid file!\n" WHITE);
        return;
    }

    dirent *entry; 
    char full_path[4096]; 
    struct stat statbuf;

    char **sub_dirs = (char **) malloc(sizeof(char *) * 256);
    if(sub_dirs == NULL) {
        printf(RED "Error : Couldn't allocate memory!\n" WHITE);
        return;
    }

    int dirs = 0;
    while ((entry = readdir(dir)) != NULL) {
        sub_dirs[dirs] = (char *) malloc(sizeof(char) * 4096);
        if(sub_dirs[dirs] == NULL) {
            printf(RED "Error : Couldn't allocate memory!\n"  WHITE);
            return;
        }
        strcpy(sub_dirs[dirs++], entry->d_name);
    }

    qsort(sub_dirs, dirs, sizeof(char *), compare_strings);

    for(int i = 0; i < dirs; i++) {
        if(path[strlen(path) - 1] == '/')
            snprintf(full_path, sizeof(full_path), "%s%s", path, sub_dirs[i]);
        else
            snprintf(full_path, sizeof(full_path), "%s/%s", path, sub_dirs[i]);
        if (stat(full_path, &statbuf) == -1) {
            // Handle error when stat fails
            continue;
        }

        if(tags & 1) {
            if((tags >> 1) & 1 == 0 && sub_dirs[i][0] == '.') {
                continue;
            }
            print_file_info(full_path, &statbuf);
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

    // Free allocated memory
    for(int i = 0; i < dirs; i++) {
        free(sub_dirs[i]);
    }
    free(sub_dirs);
}

// Function to parse commands and execute actions
void execute_reveal(dir_tree *tree, char *cmd) {
    int flags = 0;

    char **paths = (char **) malloc(sizeof(char *) * 100);
    if(paths == NULL) {
        printf(RED "Error : Couldn't allocate memory!\n" WHITE);
        // return EXIT_FAILURE;
    }
    int paths_specified = 0;

    // printf("%s\n", cmd);
    char *substr = strtok(cmd, " ");
    while(substr != NULL) {
        if(substr[0] == '-') {
            if(strlen(substr) == 1) {
                paths[paths_specified] = (char *) malloc(sizeof(char) * 4096);
                if(paths[paths_specified] == NULL) {
                    printf(RED "Error : Couldn't allocate memory!\n" WHITE);
                    // return EXIT_FAILURE;
                }
                strcpy(paths[paths_specified++], substr);
            }else {
                for(int i = 1; i < strlen(substr); i++) {
                    if(substr[i] == 'a') {
                        flags |= 2;
                    } else if(substr[i] == 'l') {
                        flags |= 1;
                    } else {
                        printf(RED "Error : Unrecognized flags passed!\n" WHITE);
                    }
                }
            }
        } else {
            paths[paths_specified] = (char *) malloc(sizeof(char) * 4096);
            if(paths[paths_specified] == NULL) {
                printf(RED "Error : Couldn't allocate memory!\n" WHITE);
                // return EXIT_FAILURE;
            }
            strcpy(paths[paths_specified++], substr);
        }

        substr = strtok(NULL, " ");
    }

    // printf("%d %d\n", paths_specified, flags);
    if(paths_specified == 0) {
        // Get the current directory path from the dir_tree
        // Convert to absolute
        char *curr_dir = get_complete_path(tree);
        // Reveal
        show(curr_dir, flags);
        free(curr_dir);
    } else {
        for(int i = 0; i < paths_specified; i++) {
            // Convert the paths to absolute
            char *abs_path = convert_to_absolute(tree, paths[i]);
            show(abs_path, flags);

            free(abs_path);
        }
    }

    free(substr);
    for(int i = 0; i < paths_specified; i ++) {
        free(paths[i]);
    }
    free(paths);
}
