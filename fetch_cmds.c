#include "fetch_cmds.h"

#define CMD_FILE ".fcom"
#define ALIAS_FILE ".myshrc"

char **fetch_commands_from_file(int **indx) {
    char **cmds = (char **) malloc(sizeof(char *) * 256);   // allow 256 commands
    for(int i = 0; i < sizeof(cmds) / sizeof(char *); i ++) {
        cmds[i] = NULL;
    }

    FILE *file = fopen(CMD_FILE, "r");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    int cmd_count = 0;
            
    char *tmp = (char *) malloc(sizeof(char) * 256);
    while (fgets(buffer, sizeof(buffer), file) != NULL) { 
        char *ptr = buffer;  
        cmds[cmd_count] = (char *) malloc(sizeof(char) * 256);
    
        sscanf(ptr, "%s %d", tmp, &((*indx)[cmd_count]));
        strcpy(cmds[cmd_count ++], tmp);
    }
    return cmds;
}