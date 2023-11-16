#include "player.h"
#include <pthread.h>

float martillo[21] = {
    392, 392, 440, 493, 493, 493, 440, 440, 440, 493, 392, 392, 392, 440, 493, 493, 493, 440, 440, 392, 392
};
worker_result_t notes[21];

int main() {
  for(int i = 0; i< 21; i++){
    notes[i].freq_1 = martillo[i];
    notes[i].freq_2 = martillo[i]/4;
    notes[i].freq_3 = martillo[i]/2;
  }
  
  player_t player;
  player_create("/dev/ttyACM0", &player);
  player_reproduce(&player, notes, 21, 500000);
  pthread_join(player.thread_handle, NULL);
  player_kill(&player);
}