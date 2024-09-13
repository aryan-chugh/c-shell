#include "handlers.h"

void restore_fds_of_terminal() {
    int dev_null = open("/dev/null", O_RDWR);
    dup2(dev_null, STDIN_FILENO);
    dup2(dev_null, STDOUT_FILENO);

    // Reopen stdin and stdout from /dev/tty
    int tty_in = open("/dev/tty", O_RDONLY);
    int tty_out = open("/dev/tty", O_WRONLY);
    dup2(tty_in, STDIN_FILENO);
    dup2(tty_out, STDOUT_FILENO);

    // Close the /dev/tty file descriptors
    close(tty_in);
    close(tty_out);
    close(dev_null);
}

void ctrl_c_handler(int signum){
    restore_fds_of_terminal();
    printf("\n");
    if(pid_foreground_curr == -1) {
        print_prompt();
        fflush(stdout);
        return;
    }

    // fprintf(stderr, "reached here %d\n", pid_foreground_curr);
    // fprintf(stderr, "%d\n", pid_foreground_curr);
    int rval = send_ping(pid_foreground_curr, SIGINT, false);
    pid_foreground_curr = -1;
    fg_comm[0] = '\0';
    fg_name[0] = '\0';
}

void ctrl_d_handler() {
    restore_fds_of_terminal();
    printf("\n");
    
    printf("\nLogging out....\n");

    if(pid_foreground_curr != -1) {
        send_ping(pid_foreground_curr, SIGKILL, false);
        pid_foreground_curr = -1;
        fg_name[0] = '\0';
        fg_comm[0] = '\0';
    }

    bg_process *bp = blist -> head_lst;
    for(int i = 0; i < blist -> num; i ++) {
        send_ping(bp ->pid, SIGKILL, false);
        bp = bp -> next;
    }

    free_bg_list(blist);

    // fprintf(stderr, "reached here %d\n", pid_foreground_curr);

    pid_foreground_curr = -1;
    fg_comm[0] = '\0';
    fg_name[0] = '\0';

    exit(EXIT_SUCCESS);
}

void ctrl_z_handler(int signum) {
    restore_fds_of_terminal();
    printf("\n");
    if(pid_foreground_curr == -1) {
        print_prompt();
        fflush(stdout);
        return;
    }
    // send to bg and stop

    // fprintf(stderr, "%d %d\n", getpgid(pid_foreground_curr), getpgid(getpid()));
    // setpgid(pid_foreground_curr, 0);

    send_ping(pid_foreground_curr, SIGTSTP, false);
    send_ping(pid_foreground_curr, SIGSTOP, false);
    // fprintf(stderr, "%d %d\n", getpgid(pid_foreground_curr), getpgid(getpid()));
    // if (setpgid(pid_foreground_curr, 0) == -1) {
    //     printf("Failed to set PGID: %s\n", strerror(errno));
    //     exit(EXIT_FAILURE);
    // }
    // send_ping(pid_foreground_curr, SIGSTOP, false);

    add_bg_process(blist, pid_foreground_curr, fg_name, fg_comm);
    fprintf(stderr, "[%d] %d\n", blist -> num, pid_foreground_curr);

    pid_foreground_curr = -1;
    fg_comm[0] = '\0';
    fg_name[0] = '\0';
}

void handle_signal(int signum) {
    int status;
    pid_t pid;

    // printf("aarha\n");
    // fprintf(stderr, "%d\n", pid);
    // bg_process *bp2 = blist -> head_lst;
    // while(bp2 != NULL) {
    //     printf("%s\n", bp2 -> command_name);
    //     bp2 = bp2 -> next;
    // }

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        bg_process *bprocess = get_process(blist, pid);
        if(bprocess == NULL) {
            // means that a foreground process came here, this does not need to be handled. 
            return;
        }

        // printf("arrha2\n");

        if(strlen(bprocess->name) == 0) {
            strcpy(bprocess -> name, "(empty)");
        }
        // fprintf(stderr, "%s\n", bprocess ->name);
        if (WIFEXITED(status)) {
            // bprocess -> status = 1;
            printf(BOLD "\r\n%s exited normally (%d)\n" RESET, bprocess->name, bprocess->pid);
            fflush(stdout);
            remove_bg_process(blist, bprocess -> pid);
        } 
        else if (WIFSIGNALED(status)) {
            // fprintf(stderr, "hello\n");
            printf(BOLD "\r\n%s exited abnormally (%d)\n" RESET, bprocess->name, bprocess->pid);
            fflush(stdout);
            remove_bg_process(blist, bprocess -> pid);
            // bprocess -> status = 2;
        }
        // Ignore if the child is stopped or continued
        else if (WIFSTOPPED(status)) {
            // Child was stopped by a signal, do not reap it
            bprocess -> status = 0;
        } 
        else if (WIFCONTINUED(status)) {
            bprocess -> status = 0;
            // if(pid_foreground_curr != -1)
            // send_ping(pid_foreground_curr, SIGCONT, false);
        }
    }

    // printBuffer();
}