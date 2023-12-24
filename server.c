#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>

#define PORT 9000
#define BUFFER_SIZE 1024

// Mutex to lock the ticket array
sem_t mutex;

// Ticket array of size 100
int tickets[100];
int ticket_remaining = 100;

// Write Log
void write_log(char* message) {
    FILE *fp;
    fp = fopen("log.txt", "a");
    fprintf(fp, "%s", message);
    fclose(fp);
}

// Function to handle ticket requests
char* buy_ticket(int ticket, int consumer_id) {
    // If ticket is available then return buy ticket successful
    if (tickets[ticket] == 0) {
        tickets[ticket] = consumer_id;
        ticket_remaining--;
        return "Buy ticket successful\n";
    }
    // If ticket is not available then return ticket not available
    return "Ticket not available\n";
}

char* return_ticket(int ticket, int consumer_id) {
    // If ticket is available then return ticket not available
    if (tickets[ticket] != consumer_id) {
        return "You do not own this ticket\n";
    }
    // If ticket is not available then return ticket successful
    tickets[ticket] = 0;
    ticket_remaining++;
    return "Return ticket successful\n";
}

char* get_ticket_status() {
    // number of tickets remaining and the tickets are still available
    char* status = malloc(100);
    sprintf(status, "Number of tickets remaining: %d\n", ticket_remaining);
    strcat(status, "Tickets: ");
    for (int i = 0; i < 100; i++) {
        if (tickets[i] == 0) {
            char ticket[10];
            sprintf(ticket, "%d ", i);
            strcat(status, ticket);
        }
    }
    strcat(status, "\n");
    return status;
}

// Function to handle ticket requests
void handle_ticket(char* str, int sock) {
    // str is the rest of the message after "handle_ticket"
    // str will be in the format "buy_ticket 1 2" or "return_ticket 1 2" or "get_ticket_status"
    // str will be tokenized by spaces
    char* token = strtok(str, " ");
    // If token is buy_ticket then buy ticket
    if (strcmp(token, "buy_ticket") == 0) {
        // Get the ticket number
        token = strtok(NULL, " ");
        int ticket = atoi(token);
        // Get the consumer id
        token = strtok(NULL, " ");
        int consumer_id = atoi(token);
        // Lock the ticket array
        sem_wait(&mutex);
        // Buy the ticket
        char* message = buy_ticket(ticket, consumer_id);
        // Write to log
        char log[1024];
        sprintf(log, "Consumer %d bought ticket %d at time %ld\n", consumer_id, ticket, time(NULL));
        write_log(log);
        // Unlock the ticket array
        sem_post(&mutex);
        // Send the message to the client
        send(sock, message, strlen(message), 0);
    }
        // If token is return_ticket then return ticket
    else if (strcmp(token, "return_ticket") == 0) {
        // Get the ticket number
        token = strtok(NULL, " ");
        int ticket = atoi(token);
        // Get the consumer id
        token = strtok(NULL, " ");
        int consumer_id = atoi(token);
        // Lock the ticket array
        sem_wait(&mutex);
        // Return the ticket
        char* message = return_ticket(ticket, consumer_id);
        // Write to log
        char log[1024];
        sprintf(log, "Consumer %d returned ticket %d at time %ld\n", consumer_id, ticket, time(NULL));
        // Unlock the ticket array
        sem_post(&mutex);
        // Send the message to the client
        send(sock, message, strlen(message), 0);
    }
        // If token is get_ticket_status then get ticket status
    else if (strcmp(token, "get_ticket_status") == 0) {
        // Lock the ticket array
        sem_wait(&mutex);
        // Get the ticket status
        char* message = get_ticket_status();
        // Write to log
        char log[1024];
        sprintf(log, "Server owner requested ticket status at time %ld\n", time(NULL));
        // Unlock the ticket array
        sem_post(&mutex);
        // Send the message to the client
        send(sock, message, strlen(message), 0);
    }
}

// Function to handle client
void *handle_client(void *socket_desc) {
    // Print thread id
    printf("Thread id: %ld\n", pthread_self());
    //Get the socket descriptor
    int sock = *(int *) socket_desc;
    ssize_t valread = 0;
    char buffer[BUFFER_SIZE] = {0};

    // Send message to the client "Accepting connections"
    char* message = "Accepting connections\n";
    send(sock, message, BUFFER_SIZE, 0);
    // Wait for client to send message until client disconnects or sends "exit"
    while (1) {
        valread = read(sock, buffer, BUFFER_SIZE - 1); // subtract 1 for the null terminator at the end
        if (valread == 0) {
            break;
        }
        if (strcmp(buffer, "exit") == 0) {
            break;
        }
        // If client sends to handle a ticket request then request begins with "handle_ticket"
        if (strncmp(buffer, "handle_ticket", 13) == 0) {
            // str to take the rest of the message after "handle_ticket"
            char* str = buffer + 14;
            printf("Request received from client: %s\n", str);

            // Handle the ticket request
            handle_ticket(str, sock);
        }
    }
    // Print thread id
    printf("Thread id: %ld\n disconnected\n", pthread_self());
    // closing the connected socket
    close(sock);

    // Exit the thread
    pthread_exit(NULL);
}

// Listen for connections and handle them
void *listen_for_connections() {
    //Get the socket descriptor
    int sock;
    int new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    // Creating socket file descriptor
    if (0 == (sock = socket(AF_INET, SOCK_STREAM, 0))) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt))) {
        perror("SocketRocket");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to address
    if (bind(sock, (struct sockaddr *)&address,
             sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(sock, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while(!0) {
        if ((new_socket = accept(sock, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        pthread_t thread_id;
        if(pthread_create(&thread_id, NULL, handle_client, &new_socket) > 0) {
            perror("could not create thread");
        }
    }
}



int main() {
    // Initialize all tickets to 0
    for (int i = 0; i < 100; i++) {
        tickets[i] = 0;
    }
    // Initialize mutex
    sem_init(&mutex, 0, 1);

    // Create a thread to listen for connections
    pthread_t thread_id;
    if(pthread_create(&thread_id, NULL, listen_for_connections, NULL) > 0) {
        perror("could not create thread");
    }
    // Take input from server owner
    while (1) {
        char input[1024];
        scanf("%s", input);
        // If input is "exit" then break
        if (strcmp(input, "exit") == 0) {
            break;
        }
        // If input is "get_ticket_status" then get ticket status
        if (strcmp(input, "get_ticket_status") == 0) {
            // Lock the ticket array
            sem_wait(&mutex);
            // Get the ticket status
            char* message = get_ticket_status();
            // Unlock the ticket array
            sem_post(&mutex);
            // Print the ticket status
            printf("%s", message);
        }
    }

    // Destroy mutex
    sem_destroy(&mutex);

    return 0;
}