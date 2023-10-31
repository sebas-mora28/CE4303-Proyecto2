#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../include/chacha20.h"
#include <sndfile.h>
#include "../include/distribute-loads.h"

#define BUFFER_SIZE 1000000

void server() {
    // Create a socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    // Set up the server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(4000); // Replace with your desired port
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the server address
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Binding failed");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server_socket, 1) == -1) {
        perror("Listening failed");
        exit(1);
    }

    while (1) {
        // Accept a client connection
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1) {
            perror("Accepting connection failed");
            exit(1);
        }

        char size_buffer[128];
        recv(client_socket, size_buffer, 128, 0);
        int size = atoi(size_buffer);

        uint8_t encrypted_audio_data[size];    
        int total_bytes_received = 0;
        char buffer[BUFFER_SIZE];
        int bytes_received = 0;

        while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) != -1) {
          int j =0;
          for(int i=total_bytes_received; i< (total_bytes_received + bytes_received)-1; i++){
            encrypted_audio_data[i] = (uint8_t)buffer[j];
            j++;
          }
          total_bytes_received += bytes_received;
          if(total_bytes_received >= size){
            break;
          }
        }


        uint8_t* audio_data = (uint8_t*)(malloc(sizeof(uint8_t) * size));
        chacha20(encrypted_audio_data, size, audio_data);


        distribute_loads(audio_data, size);
        
        // Close the client socket
        close(client_socket);
    }
    // Close the server socket (this part of the code is not reached in this example)
    close(server_socket);


};