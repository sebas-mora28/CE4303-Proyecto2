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
#include <string.h>
#include <unistd.h>

void worker(int id) {
  // No usamos el ID
  (void)id;

  server_payload_t payload;
  recv_payload(&payload);
  printf("Processing worker %d chunk_size %d fs %d\n", id, payload.size,
         payload.samplerate);

  // Allocate input and output arrays
  size_t spectrum_size = (payload.size / 2) + 1;
  double *in_chunk = (double *)fftw_malloc(sizeof(double) * payload.size);
  fftw_complex *spectrum =
      (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * spectrum_size);

  // Create a plan for the FFT
  fftw_plan plan =
      fftw_plan_dft_r2c_1d(payload.size, in_chunk, spectrum, FFTW_ESTIMATE);

  if (!plan) {
    fprintf(stderr, "Error: FFTW plan creation failed\n");
    return; // or handle the error appropriately
  }

  while (1) {
    worker_result_t result =
        get_frequencies(payload, in_chunk, spectrum, spectrum_size, plan);

    send_result(&result);
    MPI_Barrier(MPI_COMM_WORLD);

    recv_payload(&payload);
  }

  fftw_destroy_plan(plan);
  fftw_free(in_chunk);
  fftw_free(spectrum);
}
