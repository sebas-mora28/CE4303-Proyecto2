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

#define NOTE_RELATIVE_RADIUS 1.059463094359
#define NOTE_MINIMUM_MAGNITUDE 30
#define NOTE_RELATIVE_MINIMUM_MAGNITUDE 0.4
#define MAX_FREQUENCIES_DETECTED 4
#define MAX_FREQUENCY_CAP 880

void hamming(double *data, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    double hamming = 0.54 - 0.46 * cos(2.0 * M_PI * i / (size - 1));
    data[i] = hamming * data[i];
  }
}

void magnitude(fftw_complex *data, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    data[i][0] = sqrt(data[i][0] * data[i][0] + data[i][1] + data[i][1]);
  }
}

struct max_result {
  fftw_complex max_value;
  size_t max_idx;
};

struct max_result find_max_real(fftw_complex *data, size_t length) {
  struct max_result result;

  result.max_value[0] = -DBL_MAX;
  result.max_value[1] = 0;
  result.max_idx = -1;

  for (size_t i = 0; i < length; i++) {
    double inspect;
    if ((inspect = data[i][0]) > result.max_value[0]) {
      result.max_value[0] = inspect;
      result.max_idx = i;
    }
  }
  return result;
}

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

  // Array for frequencies
  float frequencies[MAX_FREQUENCIES_DETECTED];
  for (size_t i = 0; i < 600; i++) {

    // Fill the fftw_complex array with your input data
    for (int i = 0; i < payload.size; i++) {
      in_chunk[i] = payload.data[i];
    }
    if (i == 99) {
      printf("CHUNK 100 FIRST 20\n");
      for (size_t j = 0; j < 20; j++) {
        printf("\t%f\n", in_chunk[j]);
      }
    }

    // Aplicar hamming
    hamming(in_chunk, payload.size);

    // Execute the FFT
    fftw_execute(plan);

    magnitude(spectrum, spectrum_size);

    struct max_result max = find_max_real(spectrum, spectrum_size);
    double relative_minimum_magnitude =
        NOTE_RELATIVE_MINIMUM_MAGNITUDE * max.max_value[0];

    for (size_t f = 0; f < MAX_FREQUENCIES_DETECTED; f++) {
      // Reset
      frequencies[f] = 0;
    }

    // Get frequencies
    for (size_t f = 0; f < MAX_FREQUENCIES_DETECTED; f++) {
      // Get peak
      max = find_max_real(spectrum, spectrum_size);
      double freq =
          ((float)max.max_idx / (float)(payload.size - 1)) * payload.samplerate;
      double mag = max.max_value[0];
      if (mag > NOTE_MINIMUM_MAGNITUDE && mag > relative_minimum_magnitude) {
        relative_minimum_magnitude = NOTE_RELATIVE_MINIMUM_MAGNITUDE * mag;
        if (freq > MAX_FREQUENCY_CAP) {
          continue;
        }

        frequencies[f] = freq;

        // 0 spectrum around tone for next iteration
        double low_bound = freq / NOTE_RELATIVE_RADIUS;
        double high_bound = freq * NOTE_RELATIVE_RADIUS;

        long low_idx = floor((low_bound / payload.samplerate) * payload.size);
        if (low_idx < 0) {
          low_idx = 0;
        }
        size_t high_idx =
            ceil((high_bound / payload.samplerate) * payload.size);
        if (high_idx >= spectrum_size) {
          high_idx = spectrum_size;
        }

        for (size_t j = low_idx; j < high_idx; j++) {
          spectrum[j][0] = 0;
        }
      } else {
        break;
      }
    }

    worker_result_t result;
    result.freq_1 = frequencies[0];
    result.freq_2 = frequencies[1];
    result.freq_3 = frequencies[2];
    result.freq_4 = frequencies[3];

    send_result(&result);
    MPI_Barrier(MPI_COMM_WORLD);

    recv_payload(&payload);
  }

  fftw_destroy_plan(plan);
  fftw_free(in_chunk);
  fftw_free(spectrum);
}
