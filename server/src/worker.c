#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../include/types.h"
#include "../include/chacha20.h"
#include "../include/comm.h"


void worker(int id){
          
    while(1){
        
        server_payload_t payload;
        recv_payload(&payload);

        /**
            Apply filter or whatever
        */

        worker_result_t result; 
        result.freq_1 = 1;
        result.freq_2 = 2;
        result.freq_2 = 3;
        result.freq_4 = 4;

        send_result(&result);
        MPI_Barrier(MPI_COMM_WORLD);
    }
}