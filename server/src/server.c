#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../include/chacha20.h"
#include <sndfile.h>
#include "../include/distribute-loads.h"
#include "math.h"
#define BUFFER_SIZE 1000000
#define TEMP_FILE "temp.wav"


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
    printf("Listening...\n");
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
        uint8_t* encrypted_raw_data = (uint8_t*) malloc(sizeof(uint8_t) * size);    
        int total_bytes_received = 0;
        char buffer[BUFFER_SIZE];
        int bytes_received;
        while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) != 0) {

          for(int i=0; i < bytes_received; i++){
            encrypted_raw_data[total_bytes_received + i] = (uint8_t) buffer[i];
          }
          total_bytes_received += bytes_received;
        }

        uint8_t* raw_data = (uint8_t*)(malloc(sizeof(uint8_t) * size));
        chacha20(encrypted_raw_data, size, raw_data);


        FILE* file = fopen(TEMP_FILE, "w");
        fwrite(raw_data, 1, size, file);
        fclose(file);
        SNDFILE* audio_file; SF_INFO sfinfo;
        audio_file = sf_open(TEMP_FILE, SFM_READ, &sfinfo);
        int items = sfinfo.channels * sfinfo.frames;
        float* audio_data = (float*)malloc(sizeof(float) * items);
        sf_read_float(audio_file, &audio_data[0] , items);
        sf_close(audio_file);

        int channels_combined_size = items / 2;
        float* channels_combined = malloc(sizeof(float) * channels_combined_size);
        int index = 0;
        for(int i=0; i < items ; i+= 2){
            channels_combined[index] = (audio_data[i] + audio_data[i+1]) / 2;
            index++;
        }
    
        distribute_loads(channels_combined, channels_combined_size, sfinfo.samplerate);
        
        printf("End !\n");
        // Close the client socket
        close(client_socket);
    }
    // Close the server socket (this part of the code is not reached in this example)
    close(server_socket);


};