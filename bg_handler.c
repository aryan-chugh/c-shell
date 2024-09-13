#include "bg_handler.h"

bg_list *initialize_bg() {
    bg_list *lst = (bg_list *) malloc(sizeof(bg_list));
    lst -> num = 0;
    lst -> head_lst = NULL;

    return lst; 
}

void add_bg_process(bg_list *lst, int pid, char *proc_name, char *command_name) {
    bg_process *head = lst -> head_lst;
    
    bg_process *tmp = (bg_process *) malloc(sizeof(bg_process));
    if(tmp == NULL) {
        printf(RED "Error : Couldn't allocate the required memory!\n" WHITE);
        return;
    }
    tmp -> name = (char *) malloc(sizeof(char) * 4096);
    if(tmp -> name == NULL) {
        printf(RED "Error : Couldn't allocate the required memory!\n" WHITE);
        return;
    }

    tmp -> command_name = (char *) malloc(sizeof(char) * 4096);
    if(tmp -> command_name == NULL) {
        printf(RED "Error : Couldn't allocate the required memory!\n" WHITE);
        return;
    }
    tmp -> pid = pid;
    tmp -> status = 0;
    tmp -> next = NULL;
    tmp -> prev = NULL;

    strcpy(tmp -> name, proc_name);
    strcpy(tmp -> command_name, command_name);
    if(head == NULL) {
        lst -> num = lst -> num + 1;
        lst -> head_lst = tmp;
        return;
    }

    while(head -> next != NULL) {
        head = head -> next;
    }

    head -> next = tmp;
    tmp -> prev = head;

    lst -> num = lst -> num + 1;
}

// save the list after calling this function.
void remove_bg_process(bg_list *lst, int pid) {
    bg_process *head = lst -> head_lst;
    
    while(head != NULL) {
        if(head -> pid == pid) {
            if(head -> next != NULL) {
                head -> next -> prev = head -> prev;
            }
            if(head -> prev != NULL) {
                head -> prev -> next = head -> next;
            }else {
                // head is the first process in the list
                // the head of the original list will also be modified because of pass by reference.
                lst -> head_lst = head -> next;
            }

            free(head -> command_name);
            free(head -> name);
            free(head);
            lst -> num = lst -> num - 1;
            
            return;
        }
        head = head -> next;
    }
}

void free_bg_list(bg_list *lst) {
    bg_process *bp = lst->head_lst;
    bg_process *tmp;

    while (bp != NULL) {
        tmp = bp;
        bp = bp->next;

        free(tmp->command_name);  
        free(tmp->name);        
        free(tmp);
    }

    lst->head_lst = NULL;
    lst->num = 0;

    free(lst);
}

char *get_name(bg_list *lst, int pid) {
    bg_process *head = lst -> head_lst;
    
    while(head != NULL) {
        if(head -> pid == pid) {
            return head -> name;
        }
        head = head -> next;
    }
    return NULL;
}

bg_process *get_process(bg_list *lst, int pid) {
    bg_process *head = lst -> head_lst;
    
    while(head != NULL) {
        // printf("%d\n", head->pid);
        if(head -> pid == pid) {
            // printf("yes\n");
            return head;
        }
        head = head -> next;
    }
    return NULL;
}

int get_num(bg_list *lst) {
    return lst -> num;
}