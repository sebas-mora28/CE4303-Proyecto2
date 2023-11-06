

#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/types.h"

void distribute_loads(int* audio_data, int size){

        int world_size;
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);
        world_size--;
        
        int destination;
        int offset = size / world_size;
        for (int i=0; i<world_size; i++)
        {
            destination = i+1;
            int start = i * offset;
            int end = i == world_size -1 ? size - 1 : start + offset -1;
            int chunk_size = end - start + 1;

            int chunk[chunk_size]; 
            for(int j=0; j< chunk_size; j++){
                chunk[j] = audio_data[start + j];
            };

            printf("Sending to worker %d\n", destination);
            MPI_Send(&chunk_size, 1, MPI_INT, destination, 0, MPI_COMM_WORLD);
            MPI_Send(&chunk[start], chunk_size , MPI_CHAR, destination, 0, MPI_COMM_WORLD);
        }
       

        int source;
        for(int i=0; i<world_size; i++){
            source = i + 1;
            int res;
            MPI_Recv(&res, 1, MPI_INT, MPI_ANY_SOURCE , 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
            printf("Receiving result from worker %d: %d\n", source, res);       
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }   

