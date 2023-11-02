

#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/types.h"

#define MAX_WORKERS 3

void distribute_loads(int* data, int size){

        int destination;
        int offset = size / MAX_WORKERS;

        printf("Size: %d\n", size);

        for (int i=0; i<MAX_WORKERS; i++)
        {

            destination = i+1;
                
            int start = i * offset;
            int end = i == MAX_WORKERS -1 ? size - 1 : start + offset -1;
            int chunk_size = end - start + 1;

            int values[chunk_size]; 
            for(int j=0; j< chunk_size; j++){
                values[j] = data[start + j];
            };

            server_payload_t payload;
            payload.size = chunk_size;
            payload.workerId = destination;
            memcpy(payload.data, values, chunk_size * sizeof(int));

            printf("Sending to worker %d\n", destination);
            MPI_Send(&payload, sizeof(payload), MPI_BYTE, destination, 0, MPI_COMM_WORLD);
        }
       

        int source;
        for(int i=0; i<MAX_WORKERS; i++){
            source = i + 1;
            printf("Receiving result from worker %d", source);       

            //MPI_Recv(&payload, sizeof(payload), MPI_BYTE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }   

