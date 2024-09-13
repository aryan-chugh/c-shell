#include "activities.h"

int comp_function(const void *a, const void *b) {
    return *((int *) a) - *((int *) b);
}

void print_activities(bg_list *lst) {
    int pids[lst->num];

    bg_process *bp = lst->head_lst;
    for(int i = 0; i < lst->num; i ++){
        pids[i] = bp->pid;
        bp = bp -> next;
    }

    qsort(pids, lst->num, sizeof(int), comp_function);
    for(int i = 0; i < lst->num; i ++) {
        int p = pids[i];

        bg_process *tmp = get_process(lst, p);

        char st = get_status(tmp -> pid);
        printf("%d : %s - %s\n", tmp->pid, tmp->command_name, (st != 'T') ? "Running" : "Stopped");
    }
}