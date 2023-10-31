#ifndef SOCKET_H  // Include guards to prevent multiple inclusions
#define SOCKET_H


#include <stddef.h>
#include <stdint.h>


void init_client(char* ip, int port);

void send_data(uint8_t* data, size_t size);

#endif