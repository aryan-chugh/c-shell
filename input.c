#include "input.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

char *minimise_spaces(const char *inp) {
    size_t len = strlen(inp);
    char *str = (char *) malloc(sizeof(char) * (len + 1)); 
    if (!str) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    bool has_char_before = false;
    bool has_space_before = false;

    size_t trav = 0;
    for (size_t i = 0; i < len; i++) {
        if (inp[i] == ' ' || inp[i] == '\t') {
            if (has_char_before) {
                has_space_before = true;
            }
        } else {
            if (has_char_before && has_space_before) {
                str[trav++] = ' '; 
            }
            has_char_before = true;
            has_space_before = false;
            str[trav++] = inp[i];
        }
    }
    str[trav] = '\0';
    return str;
}

// del2 = & or |
// del1 = ; or \0
bool* mark_splits(char *str, char del1, char del2) {
    bool *is_bg = (bool *) malloc(sizeof(bool) * 100);
    memset(is_bg, false, sizeof(bool) * 100);
    int count = 0;

    bool is_char_before = false;
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] == del1 || str[i] == del2) {
            if (!is_char_before) {
                printf("Invalid use of %c\n", str[i]);
                // Error handling... 
            }
        } else if (str[i] != ' ' && str[i] != '\t') {
            is_char_before = true;
        }

        if (str[i] == del1) {
            is_bg[count++] = false;
        } else if (str[i] == del2) {
            is_bg[count++] = true;
        }
    }
    return is_bg;
}

char **split(char *inp, char *delim) {
    char *tok;

    char **list_of_commands = (char **) malloc(sizeof(char *) * MAX_COMMANDS);
    for(int i = 0; i < MAX_COMMANDS; i ++) {
        list_of_commands[i] = NULL;
    }

    if (!list_of_commands) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    int cmds = 0;
    tok = strtok(inp, delim);
    while (tok != NULL) { 
        char *tmp = (char *) malloc(sizeof(char) * (MAX_COMMAND_LENGTH + 1)); 
        if (!tmp) {
            perror("Failed to allocate memory");
            exit(EXIT_FAILURE);
        }
        strcpy(tmp, tok);

        list_of_commands[cmds] = minimise_spaces(tmp);
        cmds++;

        tok = strtok(NULL, delim);
        free(tmp);
    }

    return list_of_commands;
}

