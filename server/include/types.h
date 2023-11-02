#ifndef TYPES_H  // Include guards to prevent multiple inclusions
#define TYPES_H

#include <stdint.h>

#define MAX_DATA_LENGTH 4096000

typedef struct {
    int workerId;
    int size;
    uint8_t data[MAX_DATA_LENGTH];
} server_payload_t;

#endif
