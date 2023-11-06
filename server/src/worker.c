#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../include/types.h"

#define CENTRAL_NODE 0

void worker(int id){
          

    int chunk_size; 

    while(1){
        MPI_Recv(&chunk_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);    
        uint8_t chunk[chunk_size];
        MPI_Recv(&chunk, chunk_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);    

        printf("Processing worker %d chunk_size %d\n", id, chunk_size);
        /**
            Apply filter
        */
        int res = 200; // Here goes the result after applying the filter
        MPI_Send(&res, 1, MPI_INT, CENTRAL_NODE, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
    }
}