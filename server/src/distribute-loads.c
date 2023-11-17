
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
    printf("num chunks linea 28 %d\n", num_chunks);
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


    printf("chunk_sizes %d\n num_zeros: %d  size %d\n", num_chunks, num_zeros, size);

    worker_result_t* results = (worker_result_t*) malloc(sizeof(worker_result_t) * num_chunks);
    int start;
    int counter = 0;
    int results_index = 0;
    while(counter < num_chunks){
        for(int i=1; i<= NODES; i++){
            start =  counter* chunk_size;
            server_payload_t payload;
            payload.size = chunk_size;
            payload.samplerate = sample_rate;
            for(int x=0; x< chunk_size; x++){
                payload.data[x] = out[start + x];
            } 
            send_payload(&payload, i);
            counter++;
        }          
        for(int source=1; source <=NODES; source++){
            worker_result_t result;
            recv_result(&result, source); 
            printf("Worker %d: freq 1: %3.10f \t freq2: %3.10f \t freq3: %3.10f \t freq4: %3.10f\n", 
            source, 
            result.freq_1, 
            result.freq_2, 
            result.freq_3, 
            result.freq_4
            );     
            results[results_index] = result;
            results_index++;
        }
        MPI_Barrier(MPI_COMM_WORLD);
        printf("-----------------------------------------------------------\n");
    }
    printf("Processing completed successfully\n");
    player_t player;
    player_create("/dev/ttyJASON0", &player);
    player_reproduce(&player, results, num_chunks, 100000);
    pthread_join(player.thread_handle, NULL);
    player_kill(&player);
    


}   

