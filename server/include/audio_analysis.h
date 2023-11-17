#include "types.h"
#include <fftw3.h>

#define CHUNK_TIME_QUANTUM 0.1 // s

worker_result_t get_frequencies(float *chunk, int chunk_size, int samplerate,
                                double *in_chunk, fftw_complex *spectrum,
                                size_t spectrum_size, fftw_plan plan);
