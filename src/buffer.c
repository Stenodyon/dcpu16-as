#include <stdlib.h>
#include <stdio.h>

#include "buffer.h"

bin_buffer_t* buffer_make()
{
    bin_buffer_t* buffer = (bin_buffer_t*)malloc(sizeof(bin_buffer_t));
    if (!buffer)
    {
        printf("Could not allocate memory for buffer\n");
        exit(-1);
    }
    buffer->capacity = 256;
    buffer->size = 0;
    buffer->virtual_location = 0;
    buffer->data = (uint16_t*)malloc(256 * sizeof(uint16_t));
    if (!buffer->data)
    {
        printf("Could not allocate memory for buffer data\n");
        exit(-1);
    }
    return buffer;
}

void buffer_append(bin_buffer_t *buffer, uint16_t value)
{
    if (buffer->size == buffer->capacity)
    {
        buffer->data = (uint16_t*)realloc(buffer->data,
                                          buffer->capacity * 2 * sizeof(uint16_t));
        buffer->capacity *= 2;
    }
    buffer->data[buffer->size++] = value;
    ++buffer->virtual_location;
}

uint16_t * buffer_reserve(bin_buffer_t *buffer, int size)
{
    if (buffer->size + size >= buffer->capacity)
    {
        int new_capacity = buffer->capacity * 2;
        while (buffer->size + size >= new_capacity)
            new_capacity *= 2;
        buffer->data = (uint16_t*)realloc(buffer->data,
                                          new_capacity * sizeof(uint16_t));
        buffer->capacity = new_capacity;
    }
    uint16_t *ptr = buffer->data + buffer->size;
    buffer->size += size;
    buffer->virtual_location += size;
    return ptr;
}

void buffer_set(bin_buffer_t *buffer, int address, uint16_t value)
{
    int physical_address = address - (buffer->virtual_location
                                      - buffer->size);
    buffer->data[physical_address] = value;
}

void buffer_destroy(bin_buffer_t *buffer)
{
    free(buffer->data);
    free(buffer);
}
