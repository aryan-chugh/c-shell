#include "dir.h"


dir_node *create_node(char *name, bool ishome) {
    dir_node *tmp = (dir_node *) malloc(sizeof(dir_node));
    if(tmp == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        return NULL;
    }
    tmp->name = (char *) malloc(sizeof(char) * 4096);
    if(tmp->name == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        return NULL;
    }

    strcpy(tmp->name, name);

    tmp->is_home = ishome;
    tmp->num_children = 0;
    tmp->children = (dir_node **) malloc(sizeof(dir_node *) * 512); // 512 children capacity
    tmp->parent = NULL;

    return tmp;
}

void setup_tree(char *home_path, dir_tree *tree) {
    char **components = (char **) malloc(sizeof(char *) * 512); // support 512 directory levels.
    if(components == NULL) {
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        exit(EXIT_FAILURE);
    }

    char c = '/';

    int comps = 0;
    int adv = 0;

    dir_node *root = create_node("", false);
    
    char *token = strtok(home_path, "/");
    
    while (token != NULL) {
        if (strcmp(token, "") != 0) {
            components[comps] = malloc(strlen(token) + 1);
            if (components[comps] == NULL) {
                fprintf(stderr, RED);
                perror("malloc");
                fprintf(stderr, WHITE);
                exit(EXIT_FAILURE);
            }
            strcpy(components[comps], token);
            comps++;
        }
        token = strtok(NULL, "/");
    }

    tree->root = root;
    tree->prev = NULL;

    // at index i = 0; we have components[i] = ""
    for (int i = 0; i < comps; i++) {
        dir_node *tmp = create_node(components[i], i == comps - 1);
        if (tmp == NULL) {
            exit(EXIT_FAILURE);
        }
        root->children[root->num_children++] = tmp;
        tmp->parent = root;
        root = tmp;
    }
    tree->home = root;
    tree->curr = root;

    // Free components array
    for (int i = 0; i < comps; i++) {
        free(components[i]);
    }
    free(components);
}

void init_cwd(dir_tree *tree) {
    char *home_path = (char *) malloc(sizeof(char) * BUFFER_SIZE);
    if (getcwd(home_path, BUFFER_SIZE) == NULL) {
        fprintf(stderr, RED);
        perror("getcwd");
        fprintf(stderr, WHITE);
        exit(EXIT_FAILURE);
    } else {
        setup_tree(home_path, tree);
    }
    free(home_path);
}

char *get_complete_path(dir_tree *tree) {
    char **components = (char **) malloc(sizeof(char *) * 512);
    if(components == NULL){
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        return NULL;
    }
    
    dir_node *cur = tree->curr;
    int comps = 0;

    // NULL is the parent of the root node.
    while (cur != NULL) {
        components[comps++] = cur->name;
        cur = cur->parent;
    }

    char *return_path = (char *) malloc(sizeof(char) * 4096);
    if(return_path == NULL){
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        return NULL;
    }

    int size_till_now = 0;

    for (int i = comps - 1; i >= 0; i--) {
        strcpy(return_path + size_till_now, components[i]);
        size_till_now += (strlen(components[i]));

        if (i > 0)
            return_path[size_till_now++] = '/'; 
    }
    return_path[size_till_now] = '\0'; // Null-terminate the string
    free(components);

    if(strcmp(return_path, "") == 0) {
        return_path[0] = '/';
        return_path[1] = '\0';
    }
    return return_path;
}

char *get_curr_path(dir_tree *tree) {
    char **components = (char **) malloc(sizeof(char *) * 512);
    if(components == NULL){
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        return NULL;
    }
    dir_node *cur = tree->curr;
    int comps = 0;

    // NULL is the parent of the root node.
    while (cur != NULL) {
        if (cur == tree->home) {
            components[comps++] = "~";
            break;
        }
        components[comps] = (char *) malloc(sizeof(char) * 4096);
        if(components[comps] == NULL){
            fprintf(stderr, RED);
            perror("malloc");
            fprintf(stderr, WHITE);
            return NULL;
        }
        strcpy(components[comps++], cur->name);
        cur = cur->parent;
    }

    char *return_path = (char *) malloc(sizeof(char) * 4096);
    if(return_path == NULL){
        fprintf(stderr, RED);
        perror("malloc");
        fprintf(stderr, WHITE);
        return NULL;
    }

    int size_till_now = 0;

    for (int i = comps - 1; i >= 0; i--) {
        strcpy(return_path + size_till_now, components[i]);
        size_till_now += (strlen(components[i]));

        if (i > 0)
            return_path[size_till_now++] = '/'; 
    }
    return_path[size_till_now] = '\0'; // Null-terminate the string
    
    free(components);
    return return_path;
}

void hop_to_home(dir_tree *tree) {
    tree->curr = tree->home;
}

void hop_to_parent(dir_tree *tree) {
    dir_node *node = tree->curr;
    if (node->parent != NULL) {
        tree->curr = node->parent;
    }
    // if we are at root do nothing
}

void hop_to_root(dir_tree *tree) {
    tree->curr = tree->root;
}

int hop_to_folder(dir_tree *tree, char *f_name) {
    char *path_to_curr = get_complete_path(tree);
    if(path_to_curr == NULL) {
        return -1;
    }

    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s/%s", path_to_curr, f_name);

    DIR *dir = opendir(buffer);
    if (dir == NULL) {
        fprintf(stderr, RED);
        perror("opendir");
        fprintf(stderr, WHITE);

        free(path_to_curr);
        return -1;
    }

    dir_node *dnode = tree->curr;
    dir_node *tmp = NULL;

    // check for existence in the child nodes;
    for (int i = 0; i < dnode->num_children; i++) {
        if (strcmp(f_name, dnode->children[i]->name) == 0) {
            tmp = dnode->children[i];
            break;
        }
    }

    if (tmp == NULL) {
        tmp = create_node(f_name, false);
        dnode->children[dnode->num_children++] = tmp;
        tmp->parent = dnode;
    }

    tree->curr = tmp;
    closedir(dir);
    free(path_to_curr);
    return 0;
}

int hop_to_prev(dir_tree *tree) {
    if (tree->prev == NULL) {
        return -1;
    } else {
        tree->curr = tree->prev;
    }
    return 0;
}

void hop_to_cur(dir_tree *tree) {
    // Redundant, does nothing
}

int hop_to_path(dir_tree *tree, int argc, char *argv[]) {
    if (argc == 0) {
        tree->prev = tree->curr;
        hop_to_home(tree);
        return 0;
    }

    for (int i = 0; i < argc; i++) {
        char *path_i = argv[i];
        
        dir_node *temp_cur = tree->curr;
        char *token = strtok(path_i, "/");

        if (path_i[0] == '/') {
            if (token == NULL) {
                tree->prev = tree->curr;
                hop_to_root(tree);
                return 0;
            } else {
                hop_to_root(tree);
            }
        }

        while (token != NULL) {
            char *tmp = (char *) malloc(strlen(token) + 1);
            if(tmp == NULL){
                fprintf(stderr, RED);
                perror("malloc");
                fprintf(stderr, WHITE);
                return -1;
            }
            strcpy(tmp, token);

            token = strtok(NULL, "/");

            if (strcmp(tmp, "~") == 0) {
                hop_to_home(tree);
            } else if (strcmp(tmp, "..") == 0) {
                hop_to_parent(tree);
            } else if (strcmp(tmp, ".") == 0) {
                hop_to_cur(tree);
            } else if (strcmp(tmp, "-") == 0) {
                if (hop_to_prev(tree) == -1) {
                    tree->curr = temp_cur;
                    free(tmp);
                    fprintf(stderr, RED);
                    perror("Error : no previous directory");
                    fprintf(stderr, WHITE);

                    return -1;
                }
            } else {
                if (hop_to_folder(tree, tmp) == -1) {
                    tree->curr = temp_cur; // restore the current directory on failure
                    free(tmp);
                    return -1;
                }
            }
            free(tmp);
        }        
        printf("%s\n", get_complete_path(tree));
        fflush(stdout);
        tree->prev = temp_cur;
    }
    return 0;
}

char *convert_to_absolute(dir_tree *tree, char *path) {
    dir_node *temp_cur = tree->curr;
    char *token = strtok(path, "/");

    if (path[0] == '/') {
        hop_to_root(tree);
        if (token == NULL) {
            char *ret_val = get_complete_path(tree);
            tree->curr = temp_cur; // restore the current directory
            return ret_val;
        }
    }

    while (token != NULL) {
        char *tmp = (char *) malloc(strlen(token) + 1);
        if(tmp == NULL){
            fprintf(stderr, RED);
            perror("malloc");
            fprintf(stderr, WHITE);
            return NULL;
        }
        strcpy(tmp, token);

        token = strtok(NULL, "/");

        if (strcmp(tmp, "~") == 0) {
            hop_to_home(tree);
        } else if (strcmp(tmp, "..") == 0) {
            hop_to_parent(tree);
        } else if (strcmp(tmp, ".") == 0) {
            hop_to_cur(tree);
        } else if (strcmp(tmp, "-") == 0) {
            if (hop_to_prev(tree) == -1) {
                tree->curr = temp_cur;
                free(tmp);

                fprintf(stderr, RED);
                perror("Error : no previous directory");
                fprintf(stderr, WHITE);
                return NULL;
            }
        } else {
            if (hop_to_folder(tree, tmp) == -1) {
                tree->curr = temp_cur; // restore the current directory on failure
                free(tmp);
                return NULL;
            }
        }
        free(tmp);
    }
    char *ret_val = get_complete_path(tree);
    tree->curr = temp_cur; // restore the current directory
    return ret_val;
}

int execute_hop(dir_tree* tree, char *cmd) {
    char *tok = strtok(cmd, " ");
    int args = 0;
    char *argv[100];

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

    int ret_ = hop_to_path(tree, args, argv);
    if(ret_ != -1) {
        char *fpth = get_complete_path(tree);
        if (chdir(fpth) != 0) {
            fprintf(stderr, RED);
            perror("chdir failed");
            fprintf(stderr, WHITE);
            return -1;
        }
    }
    return ret_;
}