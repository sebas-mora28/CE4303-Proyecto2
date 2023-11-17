#ifndef TYPES_H // Include guards to prevent multiple inclusions
#define TYPES_H

#include <stddef.h>
#include <stdint.h>

#define MAX_DATA_LENGTH 20000000

typedef struct {
  int samplerate;
  float data[MAX_DATA_LENGTH];
  int chunk_size;
  int num_chunks;
} server_payload_t;

typedef struct {
  float freq_1;
  float freq_2;
  float freq_3;
  float freq_4;
} worker_result_t;

typedef struct {
  worker_result_t data[MAX_DATA_LENGTH];
} payload_worker_result_t;

#endif
