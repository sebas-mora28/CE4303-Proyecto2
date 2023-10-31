

#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include "../include/types.h"

#define MAX_WORKERS 3

void distribute_loads(uint8_t* data, int size){

        int destination;
        for (int i=0; i<MAX_WORKERS; i++)
        {
            destination = i+1;

            shared_data_t shared_data;
            //shared_data.size = 10;
            MPI_Send(&shared_data, sizeof(shared_data), MPI_BYTE, destination, 0, MPI_COMM_WORLD);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        printf("\nEND: This need to print after all MPI_Send/MPI_Recv has been completed\n\n");

    }   

