#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define PORT 9000
#define BUFFER_SIZE 1024

int main() {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // Creating socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // read the message from server and print it
    valread = read(sock, buffer, 1024);
    printf("%s\n", buffer);

    // get id from user
    char id[1024];
    printf("Enter your id: ");
    scanf("%s", id);
    printf("Your id is %s\n", id);

    while (1) {
        // take request from user
        char request[1024];
        printf("Enter your request: ");
        scanf("%s", request);



        // if request is "exit" then break
        if (strcmp(request, "exit") == 0) {
            send(sock, request, strlen(request), 0);
            break;
        }

        // If request is "handle_ticket" then send the request to server
        if (strcmp(request, "buy_ticket") == 0) {
            // remove extra spaces from request
            char *token = strtok(request, " ");
            char request_without_spaces[1024] = {0};

            while (token != NULL) {
                strcat(request_without_spaces, token);
                token = strtok(NULL, " ");
                if (token != NULL) {
                    strcat(request_without_spaces, " ");
                }
            }

            // add prefix "handle_ticket" to the request
            char request_with_prefix[1024];
            strcpy(request_with_prefix, "handle_ticket ");

            // add request to the prefix
            strcat(request_with_prefix, request_without_spaces);

            // add id to the request
            strcat(request_with_prefix, " ");
            strcat(request_with_prefix, id);

// send the request to server
            send(sock, request_with_prefix, strlen(request_with_prefix), 0);

// read the message from server and print it
            valread = read(sock, buffer, 1024);
            printf("%s\n", buffer);

        } else if (strcmp(request, "return_ticket") == 0) {
            // add prefix "handle_ticket" to the request
            char request_with_prefix[1024];
            strcpy(request_with_prefix, "handle_ticket ");

            // add request to the prefix
            strcat(request_with_prefix, request);

            // add id to the request
            strcat(request_with_prefix, " ");
            strcat(request_with_prefix, id);

            // send the request to server
            send(sock, request_with_prefix, strlen(request_with_prefix), 0);

            // read the message from server and print it
            valread = read(sock, buffer, 1024);
            printf("%s\n", buffer);
        } else if (strcmp(request, "get_ticket_status") == 0) {
            // add prefix "handle_ticket" to the request
            char request_with_prefix[1024];
            strcpy(request_with_prefix, "handle_ticket ");

            // add request to the prefix
            strcat(request_with_prefix, request);

            // send the request to server
            send(sock, request_with_prefix, strlen(request_with_prefix), 0);

            // read the message from server and print it
            valread = read(sock, buffer, 1024);
            printf("%s\n", buffer);
        } else {
            printf("Invalid request\n");
        }

    }
    return 0;
}