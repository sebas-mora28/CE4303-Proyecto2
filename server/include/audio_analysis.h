#include "types.h"
#include <fftw3.h>

worker_result_t get_frequencies(server_payload_t payload, double *in_chunk,
                                fftw_complex *spectrum, size_t spectrum_size,
                                fftw_plan plan);
