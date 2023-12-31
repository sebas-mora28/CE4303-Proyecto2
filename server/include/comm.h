#ifndef COMM  // Include guards to prevent multiple inclusions
#define COMM

#include "../include/types.h"
#include "../include/chacha20.h"
#include <string.h>


void send_payload(server_payload_t* payload, int destination);

void recv_payload(server_payload_t* payload);

void recv_result(payload_worker_result_t* result, int source);

void send_result(payload_worker_result_t* result);




#endif
