/*
    This file is part of dcpu16-as.

    dcpu16-as is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    dcpu16-as is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with dcpu16-as.  If not, see <https://www.gnu.org/licenses/>.
*/

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
