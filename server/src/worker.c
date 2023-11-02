#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../include/types.h"


void run_worker(){
          
    server_payload_t server_payload;
    MPI_Recv(&server_payload, sizeof(server_payload), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);    

    printf("Processing worker %d", server_payload.workerId);

    int audio_data[server_payload.size];
    memcpy(audio_data, server_payload.data, server_payload.size * sizeof(int));    

    /**
        Apply filter
    */

    // Send result to server
    MPI_Barrier(MPI_COMM_WORLD);
}