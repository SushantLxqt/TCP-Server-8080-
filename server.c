#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>

//Configuring a Struct to store the port, address and TCP connection
struct sockaddr_in address;
int main(void)
{
    int server_fd, client_a, client_b;

    //Configuring the Socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET -> IPV4 and on the baseline Protocol and SOCK_STREAM -> TCP

    //Whether check if the socket is working
    if (server_fd <0 || server_fd == -1){
        perror("Server Failed to Initialize");
        return 1;
    };

    //Adding PORT,TCP,ADDRESS
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; //To bind the socket in all available local network interfaces
    address.sin_port = htons(8080);

    // Binding the server on the port 8080

    if (bind(server_fd,(struct sockaddr*)&address, sizeof(address)) == -1){
        perror("Binding Failed");
        close(server_fd);
        return 1;
    }

    //Listen to the Client
    if (listen(server_fd, 10) == -1){ //Used 10 -> 10 client requests at once
         perror("Listening Failed");
        close(server_fd);
        return 1;
    }

    printf("Server Listening on port 8080...\n");

    //For clients 
    //client A
    client_a = accept(server_fd, NULL, NULL);

    if (client_a == -1 || client_a < 0){
        perror("Connecting ClientA Failed");
        close(server_fd);
        return 1;
    }
    printf("Client A Connected\n");

    //For Client B
    client_b = accept(server_fd, NULL, NULL);

    if (client_b == -1 || client_b < 0){
        perror("Connecting Client B Failed");
        close(server_fd);
        return 1;
    }

    printf("Client B Connected Successfully\n");

    // Messaging Buffers (HardCoding)
    char client_a_message[1024];
    char client_b_message[1024];

    //To see the reply of other Client
    char message_a[1024];
    char message_b[1024];

    // Watching from Both Clients
    while(1){
        fd_set read_fds; //fd_set is a data-type of as we have select.h ( headerfile)
        FD_ZERO(&read_fds);
        FD_SET(client_a, &read_fds);
        FD_SET(client_b, &read_fds);

        // we have to find the largest file descriptor

        int max_fd;

        if (client_a > client_b){
            max_fd = client_a;
        }
        else{
            max_fd = client_b;
        }
        
        int ready = select(max_fd + 1, &read_fds, NULL,NULL,NULL);

        if (ready == -1 || ready < 0){
            perror("Failed");
            close(server_fd);
            break;
        }

        // Client A -> Server -> Client B ( as text message service)
        if (FD_ISSET(client_a, &read_fds)) {
            ssize_t n = recv(
                client_a,
                client_a_message,
                sizeof(client_a_message) - 1,
                0
            );
            //Check the message and give it file descriptor
            if (n > 0) {
                client_a_message[n] = '\0';
                // If client A types exit then we can close the conversation

                if (strcmp(client_a_message, "exit\n") == 0 ||
                    strcmp(client_a_message, "exit") == 0) {

                    printf("Client A disconnected.\n");
                    break;
                }
                // Add "Client A: " before the message for Client B to understand the difference between messages
                int message_length = snprintf(
                    message_a,
                    sizeof(message_a),
                    "Client A: %s",
                    client_a_message
                );
                // Print the Message for Server
                printf("%s", message_a);
                // Send labeled message to Client B
                send(
                    client_b,
                    message_a,
                    message_length,
                    0
                );
            }
            else if (n == 0) {
                printf("Client A disconnected.\n");
                break;
            }
            else {
                perror("Receiving Message from Client A");
                break;
            }
        }
        // Client B -> Server -> Client A ( same as above)
        if (FD_ISSET(client_b, &read_fds)) {

            ssize_t n = recv(
                client_b,
                client_b_message,
                sizeof(client_b_message) - 1,
                0
            );
            if (n > 0) {

                client_b_message[n] = '\0';

                // Check for exit same as ClientA
                if (strcmp(client_b_message, "exit\n") == 0 ||
                    strcmp(client_b_message, "exit") == 0) {

                    printf("Client B disconnected.\n");
                    break;
                }
                // Add "Client B: " before the message ( same as above)
                int message_length = snprintf(
                    message_b,
                    sizeof(message_b),
                    "Client B: %s",
                    client_b_message
                );
                // Print on server

                printf("%s", message_b);
                // Send labeled message to Client A
                send(
                    client_a,
                    message_b,
                    message_length,
                    0
                );
            }
            else if (n == 0) {
                printf("Client B disconnected.\n");
                break;
            }
            else {
                perror("recv from Client B");
                break;
            }
        }
    }
    // Finally Closing the TCP Connection
    close(server_fd);
    close(client_a);
    close(client_b);
    return 0;
}

