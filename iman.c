#include "iman.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 80
#define BUFFER_SIZE 4096

void get_request(char *srch) {
    int sock;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char request[5120], response[4096];
    
    char *host = "man.he.net";  // Change this to the desired website

    char path[4096];
    snprintf(path, sizeof(path),
             "/?topic=%s&section=all", srch);

    // char *path = "/?topic=sleep&section=all";  // Change this to the desired path

    // Create the socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        fprintf(stderr, RED);
        perror("Socket creation failed");
        fprintf(stderr, WHITE);
        exit(EXIT_FAILURE);
    }

    // Get the server's address
    server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr, RED);
        fprintf(stderr, "Error: No such host\n");
        fprintf(stderr, WHITE);
        exit(EXIT_FAILURE);
    }

    // Setup the server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    // Use h_addr_list[0] to get the first address
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, RED);
        perror("Connection failed");
        fprintf(stderr, WHITE);
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Form the GET request
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n\r\n", path, host);

    // Send the GET request to the server
    if (send(sock, request, strlen(request), 0) < 0) {
        fprintf(stderr, RED);
        perror("Send failed");
        fprintf(stderr, WHITE);
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Read the server's response
    int bytes_received;
    int tag_count = 0;

    bool prev_also = false;
    bool should_print = false;

    int prev_n = 0;
    while ((bytes_received = recv(sock, response, sizeof(response) - 1, 0)) > 0) {
        response[bytes_received] = '\0';  // Null-terminate the response

        for(int i = 0; i < strlen(response); i ++) {
            if(response[i] == '<') tag_count ++;
            else if(response[i] == '>') tag_count --;
            else {
                // code segment to not print the header
                if(!should_print && response[i] == '\n') {
                    if(prev_also) should_print = true;
                    else prev_also = true;
                }else {
                    prev_also = false;
                }
                // ---

                if(tag_count == 0) {
                    if(should_print) {
                        // code_segment to remove all the '\n' empty lines
                        if(response[i] == '\n') {
                            if(prev_n == 0) continue;
                            prev_n = 0;
                        }else {
                            prev_n = 1;
                        }
                        // --- 
                        printf("%c", response[i]);
                    }
                }
            }
        }
    }

    if (bytes_received < 0) {
        fprintf(stderr, RED);
        perror("Receive failed");
        fprintf(stderr, WHITE);
    }

    // Close the socket
    close(sock);
}


void execute_iman(char *cmd) {
    if(cmd == NULL) {
        return;
    }

    char *sv_ptr;
    char *srch = strtok_r(cmd, " ", &sv_ptr);
    if(srch == NULL) {
        fprintf(stderr, RED);
        fprintf(stderr, "Error: No search string provided!\n");
        fprintf(stderr, WHITE);
        exit(EXIT_FAILURE);
    }

    get_request(srch);
}