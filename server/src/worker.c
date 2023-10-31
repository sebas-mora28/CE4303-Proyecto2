#include <mpi.h>
#include "../include/types.h"


void run_worker(){
          
    shared_data_t shared_data;
    MPI_Recv(&shared_data, sizeof(shared_data), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);        
    sleep(1); /*This represent many calculations that will happen here later, instead of sleep*/
    printf("Printing at Rank/Process number: %d\n", shared_data.size);
    MPI_Barrier(MPI_COMM_WORLD);
}