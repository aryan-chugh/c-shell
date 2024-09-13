#ifndef DIR_TREE_H
#define DIR_TREE_H

#include "headers.h"
#include "dir.h"

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
// dir_node *create_node(char *name, bool ishome);
// void setup_tree(char *home_path, dir_tree *tree);

void update_directories(char *new_dir, int skip_prev_update);
void initialize_home();
char* get_complete_path();
char* get_curr_path();
void hop_to_home(int should_update);
void hop_to_parent(int should_update);
void hop_to_root(int should_update);
void hop_to_folder(char *folder, int should_update);
void hop_to_prev(int should_update);
void hop_to_cur();
int hop_to_paths(int argc, char *argv[], bool should_print);
char* convert_to_absolute(char *path);
int execute_hop(char *cmd);
void set_current_directory() ;
char *get_home_path();

#endif // DIR_TREE_H
