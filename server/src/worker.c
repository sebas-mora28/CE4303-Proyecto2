#include "../include/audio_analysis.h"
#include "../include/chacha20.h"
#include "../include/comm.h"
#include "../include/types.h"

#include <fftw3.h>
#include <float.h>
#include <math.h>
#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void worker(int id) {

  payload_worker_result_t *payload_result =
      malloc(sizeof(payload_worker_result_t));

  while (1) {
    server_payload_t payload;
    recv_payload(&payload);
    printf("Processing worker %d chunk_size %d chunks %d fs %d\n", id,
           payload.chunk_size, payload.num_chunks, payload.samplerate);

    // Allocate input and output arrays
    size_t spectrum_size = (payload.chunk_size / 2) + 1;
    double *in_chunk =
        (double *)fftw_malloc(sizeof(double) * payload.chunk_size);
    fftw_complex *spectrum =
        (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * spectrum_size);

    // Create a plan for the FFT
    fftw_plan plan = fftw_plan_dft_r2c_1d(payload.chunk_size, in_chunk,
                                          spectrum, FFTW_ESTIMATE);

    if (!plan) {
      fprintf(stderr, "Error: FFTW plan creation failed\n");
      return; // or handle the error appropriately
    }

    for (size_t i = 0; i < payload.num_chunks; i++) {
      worker_result_t result = get_frequencies(
          &(payload.data[i * payload.chunk_size]), payload.chunk_size,
          payload.samplerate, in_chunk, spectrum, spectrum_size, plan);

      payload_result->data[i] = result;
    }

    fftw_destroy_plan(plan);
    fftw_free(in_chunk);
    fftw_free(spectrum);

    send_result(payload_result);
    MPI_Barrier(MPI_COMM_WORLD);
  }

  free(payload_result);
}
