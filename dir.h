#ifndef DIR_TREE_H
#define DIR_TREE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#define BUFFER_SIZE 4096
#define RESET       "\033[0m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define WHITE       "\033[37m"
#define BOLD        "\033[1m"

// Definition of the dir_node structure
typedef struct dir_node {
    char *name;
    bool is_home;
    int num_children;
    struct dir_node **children;
    struct dir_node *parent;
} dir_node;

// Definition of the dir_tree structure
typedef struct dir_tree {
    dir_node *home;
    dir_node *curr;
    dir_node *prev;
    dir_node *root;
} dir_tree;

// Function declarations
dir_node *create_node(char *name, bool ishome);
void setup_tree(char *home_path, dir_tree *tree);
void init_cwd(dir_tree *tree);
char *get_complete_path(dir_tree *tree);
char *get_curr_path(dir_tree *tree);
void hop_to_home(dir_tree *tree);
void hop_to_parent(dir_tree *tree);
void hop_to_root(dir_tree *tree);
int hop_to_folder(dir_tree *tree, char *f_name);
int hop_to_prev(dir_tree *tree);
void hop_to_cur(dir_tree *tree);
int hop_to_path(dir_tree *tree, int argc, char *argv[]);
char *convert_to_absolute(dir_tree *tree, char *path);
void execute_hop(dir_tree* tree, char *cmd);

#endif // DIR_TREE_H
