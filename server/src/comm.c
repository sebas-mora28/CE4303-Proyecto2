#include "mpi.h"
#include "../include/comm.h"
#include <stdio.h>


size_t SERVER_PAYLOAD_SIZE = sizeof(server_payload_t);
size_t WORKER_RESULT_SIZE = sizeof(worker_result_t);

#define CENTRAL_NODE 0

void send_payload(server_payload_t* payload, int destination)
{
    uint8_t data[SERVER_PAYLOAD_SIZE];
    memcpy(data, payload, SERVER_PAYLOAD_SIZE);
    uint8_t encrypt_data[SERVER_PAYLOAD_SIZE];
    chacha20(data, SERVER_PAYLOAD_SIZE, encrypt_data);
    MPI_Send(&(*payload), SERVER_PAYLOAD_SIZE, MPI_BYTE, destination, 0, MPI_COMM_WORLD);
}


void recv_payload(server_payload_t* payload)
{
    uint8_t encrypt_data[SERVER_PAYLOAD_SIZE];
    MPI_Recv(&encrypt_data, SERVER_PAYLOAD_SIZE, MPI_BYTE, CENTRAL_NODE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
    uint8_t decrypt_data[SERVER_PAYLOAD_SIZE];
    chacha20(encrypt_data, SERVER_PAYLOAD_SIZE, decrypt_data);

    *payload = *(server_payload_t*)&encrypt_data; 
}

void recv_result(worker_result_t* result, int source)
{

    uint8_t encrypted_result[WORKER_RESULT_SIZE];
    MPI_Recv(&encrypted_result, WORKER_RESULT_SIZE, MPI_BYTE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
    uint8_t dencrypted_result[WORKER_RESULT_SIZE];
    chacha20(encrypted_result, WORKER_RESULT_SIZE, dencrypted_result);
    *result = *(worker_result_t*)&dencrypted_result;
}


void send_result(worker_result_t* result)
{
    uint8_t data[WORKER_RESULT_SIZE];
    memcpy(data, result, WORKER_RESULT_SIZE);
    uint8_t encrypt_data[WORKER_RESULT_SIZE];
    chacha20(data, WORKER_RESULT_SIZE, encrypt_data);
    MPI_Send(&encrypt_data, WORKER_RESULT_SIZE, MPI_BYTE, CENTRAL_NODE, 0, MPI_COMM_WORLD);
}



