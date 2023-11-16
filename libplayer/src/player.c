#include "player.h"

#include <fcntl.h>
#include <math.h>
#include <memory.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "types.h"

char* MSG_STR = "@%d%04" PRIu32 ";";
char* ERR_STR = "@ERROR;";

// return 0 if success, -1 otherwise
int player_create(char* tty_name, player_t* player_result) {
  int fd = open(tty_name, O_RDWR | O_NOCTTY);
  if (fd < 0) {
    perror("Error opening serial communication");
  }
  struct termios tty = {};
  if (tcgetattr(fd, &tty) != 0) {
    perror("tcgetattr error");
    return -1;
  }
  cfsetospeed(&tty, B921600);
  cfsetispeed(&tty, B921600);
  // 8bit word, R enable y quitar señales de control
  // valores no explícitos pero implícitos por ser default:
  // - Sin paridad
  // - 1 Stop Bit
  // - Sin CTS/RTS
  tty.c_cflag = CS8 | CREAD | CLOCAL;
  // Valores implícitos:
  // - Modo no canónico (procesa sin necesitar \n)
  // - Sin echo
  // - Sin signal chars
  tty.c_lflag = 0;
  // Valores implícitos:
  // - Sin flow control de SW
  // - Datos en seco (caracteres no interpretados a priori)
  tty.c_iflag = 0;
  tty.c_oflag = 0;
  // llamada blocking, si no logra leer el mensaje tiene un timeout a los 500ms
  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 5;
  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    perror("error from tcsetattr");
    return -1;
  }
  memset(player_result, 0, sizeof(player_t));
  player_result->fd = fd;
  return 0;
}

static int player_send_note__(player_t* player, int channel, float freq) {
  char wbuffer[] = "@00000;";
  char rbuffer[] = "@00000;";
  sprintf(wbuffer, MSG_STR, channel, (uint32_t)roundf(freq));
  if (write(player->fd, wbuffer, sizeof(wbuffer) - 1) < 0) {
    perror("Serial device write fail");
    return -1;
  }
  if (read(player->fd, rbuffer, 7) < 0) {
    perror("Serial device read fail");
    return -1;
  }
  if (strcmp(rbuffer, ERR_STR) == 0) {
    perror("Unkown error triggered on firmware");
    return -1;
  }
  return 0;
}

static void* player_reproduce_thread__(void* args) {
  player_t* player = (player_t*)args;
  (void)args;
  size_t note_index = 0;
  while (1) {
    if(note_index >= player->notes_len){
      break;
    }
    usleep(player->time_step_us);
    switch (player->state) {
      case PLAYING:
        player_send_note__(player, 0, player->notes[note_index].freq_1);
        player_send_note__(player, 1, player->notes[note_index].freq_2);
        player_send_note__(player, 2, player->notes[note_index].freq_3);
        note_index++;
        break;
      default:
        // STOP
        break;
    }
  }
  pthread_exit(0);
  return NULL;
}

int player_reproduce(player_t* player, worker_result_t* notes, size_t notes_len,
                     uint32_t time_step_us) {
  worker_result_t* notes_copy =
      (worker_result_t*)malloc(sizeof(worker_result_t) * notes_len);
  if (notes_copy == NULL) {
    perror("Could not copy song info for reproduction");
    return -1;
  }
  memcpy(notes_copy, notes, sizeof(worker_result_t) * notes_len);
  player->notes = notes_copy;
  player->notes_len = notes_len;
  player->time_step_us = time_step_us;
  player->state = PLAYING;

  return pthread_create(&player->thread_handle, NULL, player_reproduce_thread__,
                 player);

}

int player_kill(player_t* player) {
  free(player->notes);
  return close(player->fd);
}

void hello_player() { printf("hello from the lib"); }
