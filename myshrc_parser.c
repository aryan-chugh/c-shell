#include "myshrc_parser.h"

char* find_alias(char *cmd) {
    char *home = get_home_path();
    char *path = strcat(home, "/myshrc");

    FILE *fptr;
    if((fptr = fopen(path, "r")) == NULL) {
        fprintf(stderr, "Error: couldn't read from myshrc file!\n");
        return NULL;
    }

    char buffer[4096];
    while(fgets(buffer, sizeof(buffer), fptr) != NULL) {
        if(strstr(buffer, "=") != NULL) {
            char buff2[4096];
            strcpy(buff2, buffer);

            char *token = strtok(buff2, "=");
            char *st_pos;
            if((st_pos = strstr(token, cmd)) != NULL) {
                if(st_pos[strlen(cmd)] == ' ' || st_pos[strlen(cmd)] == '=' || st_pos[strlen(cmd)] == '\t') {
                    // fprintf(stderr, "hello\n");
                    token = strtok(NULL, "\n");
                    char *ret_cmd = (char *) malloc(sizeof(char) * 4096);
                    strcpy(ret_cmd, token);
                    ret_cmd[strlen(token)] = '\0';

                    if(fclose(fptr) == EOF) {
                        fprintf(stderr, "Error: couldn't close the file!\n");
                    }
                    return ret_cmd;
                }
            }   
        } 
    }
    if(fclose(fptr) == EOF) {
        fprintf(stderr, "Error: couldn't close the file!\n");
    }
    return NULL;
}

// str has to be minimised
char **string_to_argv2(char *str) {
    char *str_copy = strdup(str);
    if (!str_copy) {
        perror("strdup");
        exit(EXIT_FAILURE);
    }

    int count = 0;
    
    char **argv = (char **) malloc(sizeof(char *) * 4096);
    argv[0] = (char *) malloc(sizeof(char) * 4096);
    int indx = 0;

    bool is_in_quotes = false;
    for(int i = 0; i < strlen(str); i ++) {
        if(is_in_quotes) {
            if(str_copy[i] == '\'' || str_copy[i] == '"') {
                is_in_quotes = false;
                continue;
            }
            argv[count][indx ++] = str_copy[i];
            continue;
        }

        if(str_copy[i] == ' ') {
            argv[count++][indx ++] = '\0';
            argv[count] = (char *) malloc(sizeof(char) * 4096);
            indx = 0;
        }else {
            if(str_copy[i] == '\'' || str_copy[i] == '"') {
                is_in_quotes = true;
                continue;
            }
            argv[count][indx ++] = str_copy[i];
        }
    }
    if(indx > 0) {
        argv[count ++][indx ++] = '\0';
    }
    argv[count] = NULL;
    return argv;
}

char* find_function(char *cmd) {
    char *home = get_home_path();
    char *path = strcat(home, "/myshrc");

    FILE *fptr;
    if((fptr = fopen(path, "r")) == NULL) {
        fprintf(stderr, RED);
        fprintf(stderr, "Error: couldn't read from myshrc file! Consider checking its presence!\n");
        fprintf(stderr, WHITE);
        return NULL;
    }

    char *big_command = (char *) malloc(sizeof(char) * 4096);
    int big_indx = 0;

    char buffer[4096];
    bool is_func_going_on = false;
    bool is_done_once = false;

    while(fgets(buffer, sizeof(buffer), fptr) != NULL) {
        // fprintf(stderr, "2. %s", buffer);

        if(is_done_once) {
            break;
        }

        char *start_func;
        if((start_func = strstr(buffer, "func")) != NULL || is_func_going_on) {
            if(start_func == NULL) 
                start_func = buffer; 
            else 
                start_func += 4;

            char *sv_ptr;

            char delim1[2];
            if(is_func_going_on) {
                delim1[0] = '\n';
            }else{
                delim1[0] = ' '; 
            }
            delim1[1] = '\0';

            char *tok = strtok_r(start_func, delim1, &sv_ptr);
            int indx = 0;

            char is_name_found = false;

            while(tok != NULL) {
                // fprintf(stderr, "%s\n", tok);
                if(is_func_going_on == true) {
                    // fprintf(stderr, "1. %s\n", tok);
                    for(int i = 0; i < strlen(tok); i ++) {
                        if(tok[i] == '{') {
                            // empty
                        }else if(tok[i] == '}') {
                            is_func_going_on = false;
                            is_done_once = true;
                            break;
                        }else {
                            big_command[big_indx ++] = tok[i];
                        }
                    }

                    if(is_func_going_on) {
                        big_command[big_indx ++] = ';';
                    }
                }else if(indx == 0) { 
                    if(strcmp(cmd, tok) == 0) {
                        is_name_found = true;
                        indx ++;
                    }else{ 
                        is_name_found = false;
                        break;
                    }
                }else if(indx == 1) {
                    if(strcmp("()", tok) == 0){     // requires no space b/w the round brackets
                        indx ++;
                        is_func_going_on = true;
                    }else{
                        is_name_found = false;
                        indx = 0;
                        break;
                    }
                }

                if(is_done_once) break;

                char delim[2];
                if(is_func_going_on) {
                    delim[0] = '\n';
                }else{
                    delim[0] = ' '; 
                }
                delim[1] = '\0';
                
                // fprintf(stderr, "%d\n", delim[0]);
                tok = strtok_r(NULL, delim, &sv_ptr);
            }
        } 
    }
    big_command[big_indx ++] = '\0';

    if(fclose(fptr) == EOF) {
        fprintf(stderr, RED);
        fprintf(stderr, "Error: couldn't close the file!\n");
        fprintf(stderr, WHITE);
    }
    return big_command;
}

char *modify_for_functions(char *cmd) {
    char found[4096];
    
    char copy[4096];
    strcpy(copy, cmd);

    char *sv_ptr;
    char *tok = strtok_r(copy, " ", &sv_ptr);

    char *tmp_buff = find_function(tok);
    if(tmp_buff == NULL) {
        return cmd;
    }

    // fprintf(stderr, "%s\n", tmp_buff);

    if(strlen(tmp_buff) > 0) {
        // tmp_buff[strlen(tmp_buff)] = ' ';
        // tok = strtok_r(NULL, "\n", &sv_ptr);

        // found = minimise_spaces(strcat(tmp_buff, tok));
        char **argv = string_to_argv2(cmd);
        for(int i = 1; argv[i] != NULL; i ++) {
            // printf("%s\n", argv[i]);
            char find[10];
            find[0] = '"';
            find[1] = '$';
            find[2] = (char) (48 + i);
            find[3] = '"';
            find[4] = '\0';

            char *st_sub;
            // fprintf(stderr, "%s\n", find);
            strcpy(found, tmp_buff);
            // fprintf(stderr, "%s\n", found);
            while((st_sub = strstr(found, find)) != NULL) {
                // fprintf(stderr, "hello\n");
                int indx = (st_sub - found);
                char *tmp_  = strdup(found + indx + 3);
                strcpy(found + indx + 1, argv[i]);
                strcpy(found + indx + 1 + strlen(argv[i]), tmp_);
            }
            strcpy(tmp_buff, found);
        }

        char *ret_var = (char *) malloc(sizeof(char) * 4096); 
        if(ret_var == NULL) {
            fprintf(stderr, RED);
            perror("malloc");
            fprintf(stderr, WHITE);

            return NULL;
        }

        int ind_ret = 0;

        bool has_char_appeared = false;

        for(int i = 0; i < strlen(found); i ++) {
            if(found[i] == ';') {
                if(has_char_appeared) {
                    ret_var[ind_ret ++] = found[i]; 
                }
                has_char_appeared = false;
                continue;
            }else 
            if(found[i] != ' ' && found[i] != '\t' && found[i] != ';') {
                has_char_appeared = true;
            }
            // fprintf(stderr, "%c %s\n", found[i], ret_var);
            ret_var[ind_ret ++] = found[i];
        }
        // fprintf(stderr, "%s\n", ret_var);

        char *duplicate_ret = minimise_spaces(ret_var);
        // fprintf(stderr, "%s\n", duplicate_ret);
        free(ret_var);
        return duplicate_ret;
    }else {
        // fprintf(stderr, "hello\n");
        return cmd;
    }
}


// send the minimised command here
char *modify_for_aliases(char *cmd) {
    char *found;
    
    char copy[4096];
    strcpy(copy, cmd);

    char *sv_ptr;
    char *tok = strtok_r(copy, " ", &sv_ptr);

    char *tmp_buff = find_alias(tok);
    if(tmp_buff != NULL) {
        tmp_buff[strlen(tmp_buff)] = ' ';
        tok = strtok_r(NULL, "\n", &sv_ptr);
        
        if(tok == NULL) {
            found = minimise_spaces(tmp_buff);
            return found;
        }else {
            found = minimise_spaces(strcat(tmp_buff, tok));
            return found;
        }
    }else {
        return cmd;
    }
}
