#ifndef PLAYER_LIB_H
#define PLAYER_LIB_H

#include "types.h"
#include <inttypes.h>
#include <pthread.h>

typedef enum{
    PLAYING, 
    STOP
}player_state_t;

typedef struct {
  // file descriptor
  int fd;
  // array of notes
  worker_result_t* notes;
  // size of notes
  size_t notes_len;
  // time step for going trough the note list, in microseconds
  uint32_t time_step_us;
  player_state_t state;
  // thread handle
  pthread_t thread_handle;
} player_t;

int player_create(char* tty_name, player_t* player_result);

int player_reproduce(player_t* player, worker_result_t* notes, size_t notes_len,
                     uint32_t time_step_us);

int player_kill(player_t* player);

void hello_player(void);

#endif
