#include "audio_analysis.h"
#include "player.h"

#include <float.h>
#include <math.h>
#include <stdlib.h>

#include <fftw3.h>
#include <sndfile.h>
#include <string.h>

#define MAX_DATA_LENGTH 512000

#define NOTE_RELATIVE_RADIUS 1.059463094359
#define NOTE_MINIMUM_MAGNITUDE 60
#define NOTE_RELATIVE_MINIMUM_MAGNITUDE 0.5
#define MAX_FREQUENCIES_DETECTED 4
#define MAX_FREQUENCY_CAP 880
#define CHUNK_TIME_QUANTUM 0.1 // s

static float *FULL_SONG;
static float *OUTPUT_SONG;
static worker_result_t *OUTPUT_SONG_RESULTS;
static size_t CHUNK_LENGTH;
static size_t NUM_CHUNKS;
static float FS;
static player_t PLAYER;
static server_payload_t *SERVER_PAYLOAD = NULL;

// TEST
// Llamar solo una vez al inicio
int load_song(const char *filename) {
  SNDFILE *sndfile;
  SF_INFO sfinfo;

  sndfile = sf_open(filename, SFM_READ, &sfinfo);
  if (!sndfile) {
    fprintf(stderr, "Error opening the file '%s': %s\n", filename,
            sf_strerror(NULL));
    return 1;
  }

  FULL_SONG = (float *)malloc(sfinfo.frames * sfinfo.channels * sizeof(float));
  sf_readf_float(sndfile, FULL_SONG, sfinfo.frames);

  // Sacar mono
  if (sfinfo.channels > 1) {
    for (int j = 0; j < sfinfo.frames; ++j) {
      for (int i = 0; i < sfinfo.channels; ++i) {
        FULL_SONG[j] += FULL_SONG[j * 2 + i];
      }
      // Normalize the averaged samples
      FULL_SONG[j] /= (float)sfinfo.channels;
    }
  }

  FS = sfinfo.samplerate;

  CHUNK_LENGTH = FS * CHUNK_TIME_QUANTUM;
  NUM_CHUNKS = ceil(sfinfo.frames / (float)CHUNK_LENGTH);

  // Pad con 0s al final
  if ((size_t)(sfinfo.frames * sfinfo.channels) != CHUNK_LENGTH * NUM_CHUNKS) {
    FULL_SONG = realloc(FULL_SONG, CHUNK_LENGTH * NUM_CHUNKS * sizeof(float));
  }
  OUTPUT_SONG = (float *)malloc(CHUNK_LENGTH * NUM_CHUNKS * sizeof(float));
  OUTPUT_SONG_RESULTS = (worker_result_t *)malloc(CHUNK_LENGTH * NUM_CHUNKS *
                                                  sizeof(worker_result_t));
  memset(OUTPUT_SONG, 0, CHUNK_LENGTH * NUM_CHUNKS * sizeof(float));
  memset(OUTPUT_SONG_RESULTS, 0,
         CHUNK_LENGTH * NUM_CHUNKS * sizeof(worker_result_t));

  for (size_t i = sfinfo.frames; i < CHUNK_LENGTH * NUM_CHUNKS; i++) {
    FULL_SONG[i] = 0;
  }

  printf("FIRST 20 AFTER 1S\n");
  for (size_t i = FS; i < (FS + 20); i++) {
    printf("\t%f\n", FULL_SONG[i]);
  }

  sf_close(sndfile);

  // Instanciar player
  player_create("/dev/ttyACM0", &PLAYER);

  return 0;
}

// TEST
void fill_chunk(float *data, size_t chunk) {
  for (size_t i = 0; i < CHUNK_LENGTH; ++i) {
    data[i] = FULL_SONG[i + CHUNK_LENGTH * chunk];
  }
}

// TEST
void recv_payload(server_payload_t **payload) {
  static size_t counter = 0;

  if (SERVER_PAYLOAD == NULL) {
    SERVER_PAYLOAD = malloc(sizeof(server_payload_t));
  }

  *payload = SERVER_PAYLOAD;

  if (counter < NUM_CHUNKS) {
    fill_chunk(&(*payload)->data[0], counter);
  } else {
    for (size_t i = 0; i < CHUNK_LENGTH; ++i) {
      (*payload)->data[i] = 0;
    }
  }

  (*payload)->size = CHUNK_LENGTH;
  (*payload)->samplerate = FS;

  counter++;
}

// TEST
// float* tone: array de tamaño CHUNK_LENGTH
void generate_tone(float *tone, float frequency, float time) {

  float factor = time / CHUNK_LENGTH;
  for (size_t i = 0; i < CHUNK_LENGTH; i++) {
    tone[i] = sin(2 * M_PI * frequency * (((float)i) * factor));
  }
}

// TEST
void send_result(worker_result_t *result) {
  static size_t counter = 0;
  float frequencies[4] = {result->freq_1, result->freq_2, result->freq_3,
                          result->freq_4};
  float *tone = malloc(CHUNK_LENGTH * sizeof(float));

  for (size_t f = 0; f < 4; f++) {
    float frequency = frequencies[f];
    if (frequency > 0) {
      printf("FREQ%zu: %g\n", f, frequency);
      generate_tone(tone, frequency, CHUNK_TIME_QUANTUM);
      for (size_t i = 0; i < CHUNK_LENGTH; i++) {
        OUTPUT_SONG[i + CHUNK_LENGTH * counter] += tone[i];
      }
    }
  }

  for (size_t i = 0; i < CHUNK_LENGTH; i++) {
    OUTPUT_SONG_RESULTS[i + CHUNK_LENGTH * counter] = *result;
  }

  counter++;
  free(tone);
}

// Llamar solo 1 vez al final
int cleanup() {
  SF_INFO sfinfo;
  sfinfo.samplerate = (int)FS;
  sfinfo.channels = 1;
  sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

  // Specify the output file name
  const char *filename = "cout.wav";

  // Open the output file for writing
  SNDFILE *outfile = sf_open(filename, SFM_WRITE, &sfinfo);
  if (!outfile) {
    fprintf(stderr, "Error opening the output file: %s\n", sf_strerror(NULL));
    return 1;
  }

  // Write the audio data to the file
  sf_writef_float(outfile, OUTPUT_SONG, CHUNK_LENGTH * NUM_CHUNKS);

  // Close the output file
  sf_close(outfile);

  printf("WAV file saved successfully: %s\n", filename);

  printf("Reproducing on motors...\n");
  player_reproduce(&PLAYER, OUTPUT_SONG_RESULTS, CHUNK_LENGTH * NUM_CHUNKS,
                   100000);
  printf("Done!\n");

  free(OUTPUT_SONG_RESULTS);
  free(OUTPUT_SONG);
  free(FULL_SONG);
  free(SERVER_PAYLOAD);

  pthread_join(PLAYER.thread_handle, NULL);
  player_kill(&PLAYER);

  return 0;
}

void test_worker() {
  server_payload_t *payload;
  recv_payload(&payload);
  printf("Processing test worker chunk_size %d fs %d\n", payload->size,
         payload->samplerate);

  // Allocate input and output arrays
  size_t spectrum_size = (payload->size / 2) + 1;
  double *in_chunk = (double *)fftw_malloc(sizeof(double) * payload->size);
  fftw_complex *spectrum =
      (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * spectrum_size);

  // Create a plan for the FFT
  fftw_plan plan =
      fftw_plan_dft_r2c_1d(payload->size, in_chunk, spectrum, FFTW_ESTIMATE);

  if (!plan) {
    fprintf(stderr, "Error: FFTW plan creation failed\n");
    return; // or handle the error appropriately
  }

  for (size_t motapod = 0; motapod < NUM_CHUNKS; motapod++) {
    worker_result_t result =
        get_frequencies(*payload, in_chunk, spectrum, spectrum_size, plan);

    send_result(&result);
    // MPI_Barrier(MPI_COMM_WORLD);

    recv_payload(&payload);
  }

  fftw_destroy_plan(plan);
  fftw_free(in_chunk);
  fftw_free(spectrum);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return 1;
  }

  if (load_song(argv[1])) {
    return 1;
  };
  test_worker();
  if (cleanup()) {
    return 1;
  }
  return 0;
}
