

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


void distribute_loads(int* audio_data, int size, int sample_rate){
    int chunk_size = (int)(QUANTUM * sample_rate);
    printf("size: %d - time_ms %d - chunk_size %d  n: %d\n", size, sample_rate, chunk_size, size / chunk_size);
    int bytes_received = 0;
    int start;
    int counter = 0;
    while(bytes_received < size){
        int nodes_used = 0;

        if(size - bytes_received < chunk_size * NODES){;
            break;
        }
        for(int i=1; i<= NODES; i++){
            printf("counter %d start %d\n", counter, start);
            start =  counter* chunk_size;
            server_payload_t payload;
            payload.size = chunk_size;
            payload.workerId = i;
            for(int x=0; x< chunk_size; x++){
                payload.data[x] = audio_data[start + x];
            } 
            send_payload(&payload, i);
            bytes_received += chunk_size;
            counter++;
            nodes_used++;
        }          
        for(int source=1; source <=nodes_used; source++){
            worker_result_t result;
            recv_result(&result, source); 
            printf("Receiving result from worker %d: %3.15f\n", source, result.freq_4);     
        }
        MPI_Barrier(MPI_COMM_WORLD);
        printf("-----------------------------------------------------------\n");
    }
    printf("Counter %d \n", counter);
}   

