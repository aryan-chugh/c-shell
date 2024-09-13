#include "neonate.h"
int t;

void enable_raw_mode(struct termios *orig_termios) {
    struct termios raw;
    tcgetattr(STDIN_FILENO, orig_termios); // Get original terminal attributes
    raw = *orig_termios;

    raw.c_lflag &= ~(ECHO | ICANON | ISIG); // Disable canonical mode, echo, and signals
    
    // raw.c_lflag |= ISIG;                    // make the signals available for handling

    raw.c_iflag &= ~(IXON | ICRNL);         // Disable software flow control and carriage return translation
    raw.c_oflag &= ~(OPOST);                // Disable output post-processing
    raw.c_cc[VMIN] = 1;                     // Minimum number of bytes before read returns
    raw.c_cc[VTIME] = 0;                    // No timeout

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); // Apply the new terminal attributes
}

// Function to disable raw mode (restore cooked mode)
void disable_raw_mode(struct termios *orig_termios) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, orig_termios); // Restore original terminal attributes
}

void print_most_recent_process() {
    struct dirent *entry;
    const char *proc_dir = "/proc";
    DIR *dir = opendir(proc_dir);

    if (dir == NULL) {
        fprintf(stderr, RED);
        perror("opendir");
        fprintf(stderr, "\r");
        fprintf(stderr, WHITE);
        return;
    }

    unsigned long long mx_starttime = 0;
    long latest_pid = -1;
    while ((entry = readdir(dir))) {
        // Check if the directory name is a PID (a number)
        char *endptr;
        long pid = strtol(entry->d_name, &endptr, 10);

        // If the directory is not a PID, skip it
        if (*endptr != '\0') {
            continue;
        }

        // Construct the path to the /proc/[pid]/stat file
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "/proc/%ld/stat", pid);

        FILE *file = fopen(filepath, "r");
        
        if (file == NULL) {
            // fprintf(stderr, "%s\r\n", filepath);
            // perror("fopen");
            // fprintf(stderr, "\r");
            // continue;

            printf("%ld\r\n\n", pid);
            closedir(dir);
            return;
        }

        char ch;
        char word[4096];
        int index = 0;

        int w_index = 1;
        while ((ch = fgetc(file)) != EOF) {
            if (ch != ' ') {
                // If the character is not a space, add it to the current word
                word[index++] = ch;
            } else if (index > 0) {
                word[index] = '\0'; 
                if(w_index == 22) {
                    char *endptr2;
                    unsigned long long st = strtoull(entry->d_name, &endptr2, 10);
                    if (*endptr2 != '\0') {
                        break;
                    }

                    if(st >= mx_starttime) {
                        mx_starttime = st;
                        latest_pid = pid;
                    }
                    break;
                }
                index = 0;  // Reset index for the next word
                w_index ++;
            }
        }
        fclose(file);
    }

    if(latest_pid != -1) {
        printf("%ld\r\n\n", latest_pid);
    }else {
        fprintf(stderr, RED);
        fprintf(stderr, "Error: Couldn't find the processes!\r\n");
        fprintf(stderr, WHITE);
    }
    closedir(dir);
}

void handle_alarm(int signum) {
    print_most_recent_process();
    alarm(t);
}

void start_printing(int time) {
    t = time;
    print_most_recent_process();
    signal(SIGALRM, handle_alarm);
    alarm(t);

    struct termios orig_termios;

    // Enter raw mode and disable signals
    enable_raw_mode(&orig_termios);

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'x') {

    }

    // Restore canonical mode before exiting
    alarm(0);
    disable_raw_mode(&orig_termios);
}

void execute_neonate(char *cmd) {
    char *sv_ptr;
    char *tok = strtok_r(cmd, " ", &sv_ptr);
    if(strcmp(tok, "-n") != 0) {
        fprintf(stderr, RED);
        fprintf(stderr, "Error : Invalid flag given!\n");
        fprintf(stderr, WHITE);
        exit(EXIT_FAILURE);
    }

    tok = strtok_r(NULL, " ", &sv_ptr);

    char *end_ptr;
    long time_interval = strtol(tok, &end_ptr, 10);
    // fprintf(stderr, "%ld\n", time_interval);
    if(*end_ptr != '\0') {
        fprintf(stderr, RED);
        fprintf(stderr, "Error: invalid time interval given!\n");
        fprintf(stderr, WHITE);
        exit(EXIT_FAILURE);
    }

    start_printing((int) time_interval);
}
