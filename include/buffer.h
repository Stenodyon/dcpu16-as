#pragma once

#ifndef BUFFER_H_
#define BUFFER_H_

#include <stdint.h>

typedef struct
{
    int capacity;
    int size;
    int virtual_location;
    uint16_t * data;
} bin_buffer_t;

bin_buffer_t* buffer_make();
void buffer_append(bin_buffer_t *buffer, uint16_t value);
uint16_t * buffer_reserve(bin_buffer_t *buffer, int size);
void buffer_set(bin_buffer_t *buffer, int address, uint16_t value);
void buffer_destroy(bin_buffer_t *buffer);

#endif //BUFFER_H_
