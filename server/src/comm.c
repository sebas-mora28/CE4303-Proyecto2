#include "../include/comm.h"
#include "mpi.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t SERVER_PAYLOAD_SIZE = sizeof(server_payload_t);
size_t WORKER_RESULT_SIZE = sizeof(payload_worker_result_t);

#define CENTRAL_NODE 0

void send_payload(server_payload_t *payload, int destination) {
  uint8_t* data = malloc(SERVER_PAYLOAD_SIZE);
  memcpy(data, payload, SERVER_PAYLOAD_SIZE);
  uint8_t* encrypt_data = malloc(SERVER_PAYLOAD_SIZE);
  chacha20(data, SERVER_PAYLOAD_SIZE, encrypt_data);
  MPI_Send(encrypt_data, SERVER_PAYLOAD_SIZE, MPI_BYTE, destination, 0,
           MPI_COMM_WORLD);

  free(data);
}

void recv_payload(server_payload_t *payload) {
  uint8_t* encrypt_data = malloc(SERVER_PAYLOAD_SIZE);
  MPI_Recv(encrypt_data, SERVER_PAYLOAD_SIZE, MPI_BYTE, CENTRAL_NODE, 0,
            MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  uint8_t* decrypt_data = malloc(SERVER_PAYLOAD_SIZE);
  chacha20(encrypt_data, SERVER_PAYLOAD_SIZE, decrypt_data);

  memcpy(payload, decrypt_data, SERVER_PAYLOAD_SIZE);
  free(encrypt_data);
}

void recv_result(payload_worker_result_t *result, int source) {

    uint8_t* encrypted_result = malloc(WORKER_RESULT_SIZE);
    MPI_Recv(encrypted_result, WORKER_RESULT_SIZE, MPI_BYTE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
    uint8_t* dencrypted_result = malloc( WORKER_RESULT_SIZE);
    chacha20(encrypted_result, WORKER_RESULT_SIZE, dencrypted_result);
    memcpy(result, dencrypted_result, WORKER_RESULT_SIZE);
    free(encrypted_result);
}

void send_result(payload_worker_result_t *result) {
  
  uint8_t* data = malloc(WORKER_RESULT_SIZE);
  memcpy(data, result, WORKER_RESULT_SIZE); 
  uint8_t* encrypt_data = malloc(sizeof(payload_worker_result_t));
  chacha20(data, WORKER_RESULT_SIZE, encrypt_data);
  MPI_Send(encrypt_data, WORKER_RESULT_SIZE, MPI_BYTE, CENTRAL_NODE, 0,
            MPI_COMM_WORLD);
  free(data);
}
