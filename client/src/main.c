
#include "../include/chacha20.h"
#include "../include/socket.h"
#include "sndfile.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT 4000

#define IP "127.0.0.1"

int main(int argc, char* argv[]){

    init_client(IP, PORT);

    char* filename = argv[1];

    FILE* file;
    if(!(file = fopen(filename, "rb"))){
        printf("Error: Couldn't open file\n");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    uint8_t* data = malloc(sizeof(uint8_t) * size);
    fread(data, 1, size, file);
    fclose(file);

    uint8_t* encrypt_data = (uint8_t*)malloc(sizeof(uint8_t) * size);       
    chacha20(data, size, encrypt_data);

    send_data(encrypt_data, size);

    return 0;
}