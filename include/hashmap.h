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

#ifndef HASHMAP_H_
#define HASHMAP_H_

#include <stdint.h>

typedef struct entry
{
    const char * name;
    uint16_t location;
} entry_t;

typedef struct bucket
{
    int capacity;
    int size;
    entry_t * entries;
} bucket_t;

typedef struct hashmap
{
    int capacity;
    int size;
    bucket_t ** buckets;
} hashmap_t;

typedef struct
{
    hashmap_t *_hashmap;
    int _bucket_index;
    int _entry_index;

    const char * key;
    uint16_t value;
} hashmap_iterator_t;

// Creates an emtpy hashmap. Caller is the owner of the returned pointer
hashmap_t* hashmap_make();

// Inserts an entry into the hashmap
void hashmap_insert(hashmap_t *hashmap, const char * name, uint16_t location);

// Returns the value indexed by `name`. Returns -1 if `name` is not in the map
int hashmap_lookup(hashmap_t *hashmap, const char * name);

// Applies fcn to all elements of the hashmap
void hashmap_apply(hashmap_t *hashmap,
                   void (*fcn)(const char * key, uint16_t value));

// Removes all the elements from the hashmap
void hashmap_clear(hashmap_t *hashmap);

// Frees the memory of the hashmap
void hashmap_destroy(hashmap_t *hashmap);


// Returns an iterator for the given hashmap
hashmap_iterator_t hashmap_iter(hashmap_t *hashmap);
int hashmap_iterator_is_end(hashmap_iterator_t *iterator);
void hashmap_iterator_next(hashmap_iterator_t *iterator);

#endif //HASHMAP_H_
