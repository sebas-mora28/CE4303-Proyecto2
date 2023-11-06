#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/server.h"
#include "../include/types.h"
#include "../include/worker.h"


int main(int argc, char** argv) 
{
    MPI_Init(&argc, &argv);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    
    // Central s
    if (world_rank == 0) 
    {
        server();
    }   
    else
    { 
      worker(world_rank);
    }

    MPI_Finalize();
    return 0;
}