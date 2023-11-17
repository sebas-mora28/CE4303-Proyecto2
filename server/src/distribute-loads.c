
#include "player.h"
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

int calculate_zeros(int size, int chunk_size, int num_chunks){
    int num_zeros = chunk_size * num_chunks - size;
    if(num_chunks % NODES != 0){
        num_zeros += chunk_size;;
    } 
    return num_zeros;
}

int calculate_chunks(int size, int chunk_size){
    int num_chunks = ceil((float) size / chunk_size);
    if(num_chunks % NODES != 0){
        num_chunks++;
    }
    return num_chunks;
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
    int num_chunks = calculate_chunks(size, chunk_size);
    int num_zeros = calculate_zeros(size, chunk_size, num_chunks);  

    float* out = (float*) malloc(sizeof(float) * (size + num_zeros)); // new size iss size + num_zeros
    fill_zeros(audio_data, size, num_zeros, out);

    int chunks_per_node =  num_chunks / NODES;

    worker_result_t* results = (worker_result_t*) malloc(sizeof(worker_result_t) * num_chunks);
    int start;
    int results_index = 0;

    int i;
    for(i=1; i<= NODES; i++){

        start =  (i - 1) * (chunk_size * chunks_per_node);

        server_payload_t* payload = malloc(sizeof(server_payload_t));
        payload->chunk_size = chunk_size;
        payload->num_chunks = chunks_per_node;
        payload->samplerate = sample_rate;
        for(int x=0; x< chunk_size * chunks_per_node; x++){
            payload->data[x] = out[start + x];
        } 
        send_payload(payload, i);
    }
    
    for(int source=1; source <=NODES; source++){
            payload_worker_result_t* result = malloc(sizeof(payload_worker_result_t));
            recv_result(result, source);  

            for(int i=0; i<chunks_per_node; i++){
                results[results_index] = result->data[i];
                results_index++;
            }
 

    }
    MPI_Barrier(MPI_COMM_WORLD);

    printf("Processing completed successfully\n");
    player_t player;
    player_create("/dev/ttyJASON0", &player);
    player_reproduce(&player, results, num_chunks, 100000);
    pthread_join(player.thread_handle, NULL);
    player_kill(&player);
    


}   

