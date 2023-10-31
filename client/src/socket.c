#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int client_socket;
struct sockaddr_in server_address;

void toString(int val, char* out){
    sprintf(out, "%d", val);
}

void init_client(char* ip, int port){
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    // Create a socket
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    // Set up the server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port); // Replace with the server's port
    server_address.sin_addr.s_addr = inet_addr(ip); // Replace with the server's IP
}

void send_data(uint8_t* data, size_t size) {

        
    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Connection failed");
        exit(1);
    }

    int bytes_sent;
    char size_string[128];
    toString(size, size_string);
    bytes_sent = send(client_socket, size_string, 128, 0);
    if (bytes_sent == -1) {
        perror("Send failed");
        exit(1);
    }
    
    // Send the data
    printf("Sending data...\n");
    bytes_sent = send(client_socket, data, size, 0);
    if (bytes_sent == -1) {
        perror("Send failed");
        exit(1);
    }

    // Close the socket
    close(client_socket);
}