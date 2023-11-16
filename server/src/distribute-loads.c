

#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/types.h"
#include "../include/chacha20.h"
#include "../include/comm.h"
#include <math.h>

#define QUANTUM 0.1 // 100 ms

#define NODES 2

int zeros_amount(int size, int chunk_size, int* num_chunks){
    int num_zeros = chunk_size * *num_chunks - size;
    if(*num_chunks % NODES != 0){
        num_zeros += chunk_size;
        (*num_chunks)++;
    } 
    return num_zeros;
}

void fill_zeros(float * audio_data, int size, int num_zeros, float* out){

    for(int i=0; i< size; i++){
        out[i] = audio_data[i];
    }

    for(int j=0; j < num_zeros; j++){
        out[size + j] = 0.0;
    }
}



void distribute_loads(float* audio_data, int size, int sample_rate){
    int chunk_size = (int)(QUANTUM * sample_rate);
    int num_chunks = ceil((float) size / chunk_size);
    int num_zeros = zeros_amount(size, chunk_size, &num_chunks);  
    float* out = (float*) malloc(sizeof(float) * (size + num_zeros));
    fill_zeros(audio_data, size, chunk_size, out);
    int start;
    int counter = 0;
    while(counter < num_chunks){

        for(int i=1; i<= NODES; i++){
            start =  counter* chunk_size;
            server_payload_t payload;
            payload.size = chunk_size;
            for(int x=0; x< chunk_size; x++){
                payload.data[x] = out[start + x];
            } 
            send_payload(&payload, i);
            counter++;
        }          
        for(int source=1; source <=NODES; source++){
            worker_result_t result;
            recv_result(&result, source); 
            printf("Receiving result from worker %d: %3.15f\n", source, result.freq_4);     
        }
        MPI_Barrier(MPI_COMM_WORLD);
        printf("-----------------------------------------------------------\n");
    }
    printf("Counter %d \n", counter);
}   

