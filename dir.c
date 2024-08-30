#include "dir.h"


dir_node *create_node(char *name, bool ishome) {
    dir_node *tmp = (dir_node *) malloc(sizeof(dir_node));
    tmp->name = (char *) malloc(sizeof(char) * 100); // 100 character folder name
    strcpy(tmp->name, name);

    tmp->is_home = ishome;
    tmp->num_children = 0;
    tmp->children = (dir_node **) malloc(sizeof(dir_node *) * 10); // 10 children capacity
    tmp->parent = NULL;

    return tmp;
}

void setup_tree(char *home_path, dir_tree *tree) {
    char **components = (char **) malloc(sizeof(char *) * 100); // support 100 directory levels.
    char c = '/';

    int comps = 0;
    int adv = 0;

    dir_node *root = create_node("", false);
    
    char *token = strtok(home_path, "/");
    
    while (token != NULL) {
        if (strcmp(token, "") != 0) {
            components[comps] = malloc(strlen(token) + 1);
            if (components[comps] == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
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
        // failure
        exit(1);
    } else {
        setup_tree(home_path, tree);
    }
    free(home_path);
}

char *get_complete_path(dir_tree *tree) {
    char **components = (char **) malloc(sizeof(char *) * 100);
    
    dir_node *cur = tree->curr;
    int comps = 0;

    // NULL is the parent of the root node.
    while (cur != NULL) {
        components[comps++] = cur->name;
        cur = cur->parent;
    }

    char *return_path = (char *) malloc(sizeof(char) * BUFFER_SIZE);
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

char *get_curr_path(dir_tree *tree) {
    char **components = (char **) malloc(sizeof(char *) * 100);
    
    dir_node *cur = tree->curr;
    int comps = 0;

    // NULL is the parent of the root node.
    while (cur != NULL) {
        if (cur == tree->home) {
            components[comps++] = "~";
            break;
        }
        components[comps] = (char *) malloc(sizeof(char) * 1024);
        strcpy(components[comps++], cur->name);
        cur = cur->parent;
    }

    char *return_path = (char *) malloc(sizeof(char) * BUFFER_SIZE);
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

    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s/%s", path_to_curr, f_name);

    DIR *dir = opendir(buffer);

    if (dir == NULL) {
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

void execute_hop(dir_tree* tree, char *cmd) {
    char *tok = strtok(cmd, " ");
    int args = 0;
    char *argv[100];

    while(tok != NULL) {
        argv[args] = (char *) malloc(sizeof(char) * 4096);
        strcpy(argv[args ++], tok);
        tok = strtok(NULL, " ");
    }

    hop_to_path(tree, args, argv);
}